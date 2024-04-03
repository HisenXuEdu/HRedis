#include <iostream>
#include <string>
#include "buttonrpc.hpp"

int main() {
    string hostName = "127.0.0.1";
    int port = 5555;

    buttonrpc client;
    client.as_client(hostName, port);
    client.set_timeout(2000);

    string message;
    while(true){
        //发送数据
        std::cout << hostName << ":" << port << "> ";
        std::getline(std::cin, message);
        // string res = client.call<string>("redis_command", message).val();
        buttonrpc::value_t<string> res=client.call<string>("redis_command", message);
        if(res.error_code()){
            cout<< res.error_msg()<<std::endl;
        }
        
        //添加结束字符 
        // if(res.find("stop") != std::string::npos){
        //     break;
        // }
        else cout<< res.val()<<std::endl;
    }
    return 0;
}
