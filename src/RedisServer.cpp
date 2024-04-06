#include"RedisServer.h"

RedisServer::RedisServer(int port, const std::string& logoFilePath) 
: port(port), logoFilePath(logoFilePath), flyweightFactory(new ParserFlyweightFactory()){
    pid = getpid();
}

RedisServer* RedisServer::getInstance(){
    static RedisServer redisserver;
    return &redisserver;
}

void RedisServer::start(){
    signal(SIGINT,signalHandler);
    printLogo();
    printStartMessage();
}

void RedisServer::printLogo(){
    std::ifstream ifs(logoFilePath);
    if(!ifs.is_open()){
        std::cout<<"logoFilePath不存在"<<std::endl;
    }
    std::string line = "";
    while (std::getline(ifs, line)) {
        replaceText(line, "PORT", std::to_string(port));
        replaceText(line, "PTHREAD_ID", std::to_string(pid));
        std::cout << line << std::endl;
    }
}

 //替换字符串中的指定字符
void RedisServer::replaceText(std::string &text, const std::string &toReplaceText, const std::string &newText) {
    size_t start_pos = text.find(toReplaceText); 
    while (start_pos != std::string::npos) {
        text.replace(start_pos, toReplaceText.length(), newText);
        start_pos = text.find(toReplaceText, start_pos + newText.length());
    }
}

void RedisServer::printStartMessage(){
    std::string startMessage = "[PID] DATE # Server started.";
    std::string initMessage = "[PID] DATE * The server is now ready to accept connections on port PORT";
    
    replaceText(startMessage, "PID", std::to_string(pid));
    replaceText(startMessage, "DATE", getDate());
    replaceText(initMessage, "PORT", std::to_string(port));
    replaceText(initMessage, "PID", std::to_string(pid));
    replaceText(initMessage, "DATE", getDate());
    
    std::cout << startMessage << std::endl;
    std::cout << initMessage << std::endl;
}

//获取当前时间
std::string RedisServer::getDate() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);

    std::tm local_tm;
    localtime_r(&now_c, &local_tm); 

    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void RedisServer::signalHandler(int sig){
    //刷盘
    if (sig == SIGINT) {
        //刷盘
        
        std::cout<<"quit!"<<std::endl;
        exit(0);
    }
}

std::string RedisServer::executeTransaction(std::queue<std::string>&commandsQueue){
    
}

std::string RedisServer::handleClient(std::string receivedData){
    size_t bytesRead = receivedData.length();
    if(bytesRead>0){
        std::istringstream iss(receivedData);
        std::string command;
        std::vector<std::string> tokens;
        while(iss>>command){ //以字符串分割
            tokens.emplace_back(command);
        }

        if(!tokens.empty()){
            command = tokens.front();
            std::string responseMessage;
            if(command=="quit" || command=="exit"){
                responseMessage="stop";
                return responseMessage;
            }
            else if (command == "multi") {}   //开启事务
            else if (command == "exec") {}    //提交事务
            else if (command == "discard") {}  //放弃事务
            else {
                //处理常规指令
                if (!startMulti) {
                    std::shared_ptr<CommandParser> commandParser = flyweightFactory->getParser(command);
                    if (commandParser == nullptr) {
                        responseMessage = "Error: Command '" + command + "' not recognized.";
                    }
                    else{
                        //执行命令
                         try {
                             responseMessage = commandParser->parse(tokens);
                         }
                         catch (const std::exception& e) {
                             responseMessage = "Error processing command '" + command + "': " + e.what();
                         }
                    }
                    // 发送响应消息回客户端
                    return responseMessage;
                }
                else{
                    //添加到事物队列中
                }
            }
        }
        else {
            // 在非阻塞模式下，没有数据可读时继续循环
            return "nil";
        }

        return "err";
    }

}
