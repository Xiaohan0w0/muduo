#pragma once
#include "noncopyable.h"
#include <memory>

class Channel;
class Eventloop;
class Socket;

class TcpConnection:Noncopyable ,public std::enable_shared_from_this<TcpConnection>
{ 
    
};

