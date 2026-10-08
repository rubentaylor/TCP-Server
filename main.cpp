#include "ClientChat.h"
//Port: 54000, local use 

int main()
{
	ClientChat chat("0.0.0.0",54000);
	if (chat.initializer() != 0){
		return 1;
	}
	chat.run(); 
}