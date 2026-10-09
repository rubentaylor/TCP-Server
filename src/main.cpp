#include "ClientChat.h"

int main()
{
	ClientChat chat("0.0.0.0", 54000);
	if (chat.initializer() != 0)
	{
		return 1;
	}
	chat.run();
}