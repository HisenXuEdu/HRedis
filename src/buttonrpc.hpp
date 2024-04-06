#pragma once
#include <string>
#include <zmq.hpp>
#include <map>
#include <sstream>
#include "Serializer.hpp"
#include <iostream>
#include <functional>

//模板的别名，需要一个外敷类
// type_xx<int>::type a = 10;
template <typename T>   //一个萃取机，用来萃取T的类型。
struct type_xx {
    typedef T type;
};

//特例化的模板，当T为void时，type为int8_t
template<>
struct type_xx<void> {
    typedef int8_t type;
};

class buttonrpc{

public:
    enum rpc_role{
        RPC_CLIENT,
        RPC_SERVER
    };

    enum rpc_err_code{
		RPC_ERR_SUCCESS = 0, //成功
		RPC_ERR_FUNCTIION_NOT_BIND,//函数未绑定
		RPC_ERR_RECV_TIMEOUT //接收超时
	};

    
    template<typename T>  //rpc用来消息传输的结构体，之后序列化的就是他
    class value_t{
	public:
		typedef typename type_xx<T>::type type; //通过typename告诉编译器type_xx<T>::type 是一个类型，相当于T重命名为type
		typedef std::string msg_type;
		typedef uint16_t code_type;

		value_t() { code_ = 0; msg_.clear(); } 
        bool valid() { return (code_ == 0 ? true : false); } //判断是否有效
        int error_code() { return code_; } //返回错误码
        std::string error_msg() { return msg_; } 
        type val() { return val_; }	//返回值


        void set_val(const type& val) { val_ = val; }
        void set_code(code_type code) { code_ = code; }    
        void set_msg(msg_type msg) { msg_ = msg; }

		friend Serializer& operator >> (Serializer& in, value_t<T>& d) { //定义友元函数    //done 这两个友元函数好像并不会被调用啊，会被调用，这个bug找了很久！！！
            in >> d.code_ >> d.msg_; 
			if (d.code_ == 0) {
				in >> d.val_;
			}
			return in;
        }
		
		friend Serializer& operator << (Serializer& out, value_t<T> d) {
			out << d.code_ << d.msg_ << d.val_; //重载运算符<< 
			return out;
		}


	private:
        code_type code_;  //这个是传输过程中的状态标识
		msg_type msg_;  //
		type val_;   //这个是调用完后的函数返回值，比如这个Redis就是“OK”
	};

    buttonrpc();
	~buttonrpc();

    // network
    void as_client(std::string ip, int port); //客户端
	void as_server(int port); //服务器
	void send(zmq::message_t& data); //发送数据
	void recv(zmq::message_t& data);//接收数据
	void set_timeout(uint32_t ms);//设置超时时间
	void run();


public:
    //server
    template<typename F, typename S>
	void bind(std::string name, F func, S* s); //类成员函数
	template<typename F, typename S>
	void callproxy(F fun, S* s, Serializer* pr, const char* data, int len); //类成员函数

    // client
    template<typename R, typename P1>
	value_t<R> call(std::string name, P1); //一个参数

private:
	//server内部进行函数调用
    Serializer* call_(std::string name, const char* data, int len);

    template<typename R>
	value_t<R> net_call(Serializer& ds);

	// PROXY CLASS MEMBER，function不能包装类成员变量或函数，需要配合Bind,传入函数地址和类对象地址
    template<typename R, typename C, typename S, typename P1>
	void callproxy_(R(C::* func)(P1), S* s, Serializer* pr, const char* data, int len) {
		//func 是类的成员函数，C是所属的类，R是返回类型，P1是参数类型
		callproxy_(std::function<R(P1)>(std::bind(func, s, std::placeholders::_1)), pr, data, len);
		//这里s作为func的第一参数传入，而类成员函数第一个参数是this指针，所以传入的应该是指向该对象的指针
	}

	// PORXY FUNCTIONAL
    template<typename R, typename P1>
	void callproxy_(std::function<R(P1)>, Serializer* pr, const char* data, int len); //实际的调用函数，前序的都是通过bind等操作把参数统一成相同数量的。


private:
    std::map<std::string, std::function<void(Serializer*, const char*, int)>> m_handlers; //函数映射表
    
    zmq::context_t m_context; //上下文
    zmq::socket_t* m_socket; //套接字

    rpc_err_code m_error_code; //错误码
    int m_role; //角色
};

buttonrpc::buttonrpc() : m_context(1){ 
	// m_error_code = RPC_ERR_SUCCESS; 
}

buttonrpc::~buttonrpc(){ 
	m_socket->close(); //关闭套接字
	delete m_socket;
	m_context.close(); //关闭上下文
}

void buttonrpc::recv(zmq::message_t& data){
	m_socket->recv(&data);
}

void buttonrpc::send(zmq::message_t& data )
{
	m_socket->send(data);  //发送数据
}



void buttonrpc::as_server( int port )
{
	m_role = RPC_SERVER; //设置角色为服务器
	m_socket = new zmq::socket_t(m_context, ZMQ_REP); //创建一个套接字 参数为上下文和套接字类型	 //ZMQ_REP 用于请求-应答模式
	std::ostringstream os;
	os << "tcp://*:" << port;
	m_socket->bind (os.str()); //绑定到指定的地址
}

template<typename F, typename S>
inline void buttonrpc::bind(std::string name, F func, S* s) //类函数，注册类函数成为远程调用的函数
{
	m_handlers[name] = std::bind(&buttonrpc::callproxy<F, S>, this, func, s, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//它是一个函数适配器，接受一个可调用对象（callable object），生成一个新的可调用对象来“适应”原对象的参数列表。因为是成员函数，所以要传递this指针
	//这个函数变成了传递三个参数的函数接口placeholders
}

template<typename F, typename S>
inline void buttonrpc::callproxy(F fun, S * s, Serializer * pr, const char * data, int len)//代理类函数
{
	callproxy_(fun, s, pr, data, len);
}



// #pragma region 区分返回值   //??? 这里不懂
// // help call return value type is void function ,c++11的模板参数类型约束
// template<typename R, typename F>
// typename std::enable_if<std::is_same<R, void>::value, typename type_xx<R>::type >::type call_helper(F f) {
// 	f();
// 	return 0;
// }
// template<typename R, typename F>
// typename std::enable_if<!std::is_same<R, void>::value, typename type_xx<R>::type >::type call_helper(F f) {
// 	return f();
// }
// #pragma endregion





template<typename R, typename P1>
void buttonrpc::callproxy_(std::function<R(P1)> func, Serializer* pr, const char* data, int len)
{
	/*
    typename关键字用于指定一个依赖类型,依赖类型是指在模板参数中定义的类型，其具体类型直到模板实例化时才能确定。
    ype_xx<R>::type是一个依赖类型，因为它依赖于模板参数R。在这种情况下，你需要使用typename关键字来告诉编译器type_xx<R>::type是一个类型。
    如果不使用typename，编译器可能会将type_xx<R>::type解析为一个静态成员
    */
	Serializer ds(StreamBuffer(data, len));
	P1 p1;
	ds >> p1;
	// typename type_xx<R>::type r = call_helper<R>(std::bind(func, p1));  //done 这里调用call_helper就可以，直接调用函数就会段错误 | 一样的，段错误是因为我set函数没写返回值，相当于string没有被赋初值。
	typename type_xx<R>::type r=func(p1);

	value_t<R> val;
	val.set_code(RPC_ERR_SUCCESS);
	val.set_val(r);
	(*pr) << val; //这个也是先调用value_t的友元函数，之前因为没写所以有bug
}

void buttonrpc::run()
{
    if (m_role != RPC_SERVER) { //如果不是服务器
		return;
	}
	while (1){
		zmq::message_t data;  //创建一个消息
		recv(data); //接收数据 没消息就阻塞
		StreamBuffer iodev((char*)data.data(), data.size());//创建一个流缓冲区
		Serializer ds(iodev); //创建一个序列化器

		std::string funname;
		ds >> funname; //读取函数名，client传来的是两个string进行的序列化，第一个是函数名
		Serializer* r = call_(funname, ds.current(), ds.size()- funname.size()); //调用函数

		zmq::message_t retmsg (r->size()); //创建一个消息
		memcpy (retmsg.data (), r->data(), r->size()); //拷贝数据，memcpy要指定拷贝多少字节
		send(retmsg); //发送数据
		delete r;  //防止内存泄漏
	}
}

// 处理函数相关
Serializer* buttonrpc::call_(std::string name, const char* data, int len)
{
    Serializer* ds = new Serializer(); //创建一个序列化器
    if (m_handlers.find(name) == m_handlers.end()) { //如果没有找到函数
		(*ds) << value_t<int>::code_type(RPC_ERR_FUNCTIION_NOT_BIND); //设置错误码
		(*ds) << value_t<int>::msg_type("function not bind: " + name); //设置错误信息
		//val_就没有写入了，因为出错了没有val_
		return ds;
	}

    auto fun = m_handlers[name]; //获取函数
    fun(ds, data, len);  //调用函数，通过统一的三个参数接口进行调用
    ds->reset(); //重置序列号容器
    return ds;
}

//server

void buttonrpc::as_client( std::string ip, int port )
{
    m_role = RPC_CLIENT;
    m_socket = new zmq::socket_t(m_context, ZMQ_REQ); //创建一个套接字 参数为上下文和套接字类型	 //ZMQ_REQ 用于请求-应答模式
    std::ostringstream os;//创建一个字符串流
    os << "tcp://" << ip << ":" << port;
    m_socket->connect (os.str()); //连接到指定的地址
    
}

inline void buttonrpc::set_timeout(uint32_t ms)
{
	// only client can set
	if (m_role == RPC_CLIENT) {
		m_socket->setsockopt(ZMQ_RCVTIMEO, ms); //设置接收超时时间
	}
	//该选项用于设置 ZeroMQ 套接字的接收超时时间。参数 ms 表示以毫秒为单位的超时时间。服务端没开启会迅速返回。
}

template<typename R>
inline buttonrpc::value_t<R> buttonrpc::net_call(Serializer& ds)
{
	zmq::message_t request(ds.size() + 1);
	memcpy(request.data(), ds.data(), ds.size());
	if (m_error_code != RPC_ERR_RECV_TIMEOUT) {
		send(request);
	}
	zmq::message_t reply;
	recv(reply);
	value_t<R> val;
	if(reply.size()==0){
		// timeout
		m_error_code = RPC_ERR_RECV_TIMEOUT;
		val.set_code(RPC_ERR_RECV_TIMEOUT);
		val.set_msg("recv timeout");
		return val;
	}
	m_error_code = RPC_ERR_SUCCESS;
	ds.clear();
	ds.write_raw_data((char*)reply.data(), reply.size());
	ds.reset();

	ds >> val;
	return val;
}

template<typename R, typename P1>
inline buttonrpc::value_t<R> buttonrpc::call(std::string name, P1 p1)
{
	Serializer ds;
	ds << name << p1;
	return net_call<R>(ds);
}
