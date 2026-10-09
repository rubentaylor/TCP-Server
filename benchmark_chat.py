"""Measure local sender-to-recipient delivery latency for the chat server."""

import argparse
import math
import selectors
import socket
import statistics
import time

WELCOME = b"Welcome to the server! Please enter your username: "

'''
command line usage:
python3 benchmark_chat.py --clients 500 --count 500  
'''

def read_until(sock: socket.socket, marker: bytes) -> bytes:
    received = bytearray()
    while marker not in received:
        chunk = sock.recv(1024)
        if not chunk:
            raise ConnectionError("Server closed the connection unexpectedly")
        received.extend(chunk)
    return bytes(received)


def percentile(samples: list[float], percent: float) -> float:
    ordered = sorted(samples)
    index = max(0, math.ceil(percent * len(ordered)) - 1)
    return ordered[index]


def connect_client(host: str, port: int, username: str) -> socket.socket:
    client = socket.create_connection((host, port), timeout=5)
    client.settimeout(5)
    read_until(client, WELCOME)
    client.sendall((username + "\n").encode())
    return client


def collect_lines(
    selector: selectors.BaseSelector,
    buffers: dict[socket.socket, bytearray],
    client_numbers: dict[socket.socket, int],
    expected_count: int,
    on_line,
) -> None:
    received_count = 0
    deadline = time.monotonic() + 30
    while received_count < expected_count:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise TimeoutError(f"Timed out after receiving {received_count}/{expected_count} messages")
        ready_sockets = selector.select(remaining)
        if not ready_sockets:
            raise TimeoutError(f"Timed out after receiving {received_count}/{expected_count} messages")

        for key, _ in ready_sockets:
            client = key.fileobj
            data = client.recv(65536)
            if not data:
                raise ConnectionError(f"Client {client_numbers[client]} disconnected during the test")

            buffers[client].extend(data)
            received_at = time.perf_counter_ns()
            while b"\n" in buffers[client]:
                line, _, rest = buffers[client].partition(b"\n")
                buffers[client] = bytearray(rest)
                on_line(client_numbers[client], line + b"\n", received_at)
                received_count += 1
                if received_count == expected_count:
                    return


def open_clients(host: str, port: int, count: int):
    clients = []
    selector = selectors.DefaultSelector()
    buffers = {}
    client_numbers = {}
    try:
        for client_number in range(count):
            client = connect_client(host, port, f"LatencyClient{client_number:04d}")
            client.setblocking(False)
            clients.append(client)
            buffers[client] = bytearray()
            client_numbers[client] = client_number
            selector.register(client, selectors.EVENT_READ)

            # Wait until all earlier clients received this join notice before
            # adding another client, keeping setup messages out of the test.
            def check_join_notice(_recipient: int, line: bytes, _received_at: int) -> None:
                if not line.endswith(b" has joined the chat.\n"):
                    raise RuntimeError(f"Unexpected message while setting up clients: {line!r}")

            if client_number > 0:
                collect_lines(
                    selector,
                    buffers,
                    client_numbers,
                    client_number,
                    check_join_notice,
                )
        return clients, selector, buffers, client_numbers
    except Exception:
        for client in clients:
            client.close()
        selector.close()
        raise


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=54000)
    parser.add_argument("--count", type=int, default=1000)
    parser.add_argument("--clients", type=int, default=2, help="simultaneous chat clients (minimum 2)")
    args = parser.parse_args()
    if args.count < 1:
        parser.error("--count must be at least 1")
    if args.clients < 2:
        parser.error("--clients must be at least 2")

    clients = []
    selector = None
    try:
        clients, selector, buffers, client_numbers = open_clients(args.host, args.port, args.clients)
        sender = clients[0]
        expected_line_prefix = b"LatencyClient0000: "

        latencies_ms = []
        start_time = time.perf_counter()
        for message_number in range(args.count):
            message = f"latency-probe-{message_number}\n".encode()
            sent_at = time.perf_counter_ns()
            sender.sendall(message)

            expected_line = expected_line_prefix + f"latency-probe-{message_number}\n".encode()

            def record_delivery(_client_number: int, line: bytes, received_at: int) -> None:
                if line != expected_line:
                    raise RuntimeError(f"Expected {expected_line!r}, received {line!r}")
                latencies_ms.append((received_at - sent_at) / 1_000_000)

            collect_lines(
                selector,
                buffers,
                client_numbers,
                args.clients - 1,
                record_delivery,
            )

        elapsed_seconds = time.perf_counter() - start_time
        delivery_count = len(latencies_ms)
        print(f"Clients: {args.clients}")
        print(f"Broadcasts: {args.count}/{args.count}")
        print(f"Recipient deliveries: {delivery_count}/{args.count * (args.clients - 1)}")
        print(f"Elapsed: {elapsed_seconds:.3f} s")
        print(f"Recipient deliveries: {delivery_count / elapsed_seconds:.1f} messages/s")
        print(f"Latency p50: {statistics.median(latencies_ms):.3f} ms")
        print(f"Latency p95: {percentile(latencies_ms, 0.95):.3f} ms")
        print(f"Latency p99: {percentile(latencies_ms, 0.99):.3f} ms")
        print(f"Latency min/max: {min(latencies_ms):.3f}/{max(latencies_ms):.3f} ms")
    finally:
        for client in clients:
            client.close()
        if selector is not None:
            selector.close()


if __name__ == "__main__":
    main()
