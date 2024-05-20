#include "ClientChat.h"
//Port: 54000, local use 

int main()
{
	ClientChat cc("0.0.0.0",54000);
	if (cc.initializer() != 0){
		return 1;
	}
	cc.run(); 
}