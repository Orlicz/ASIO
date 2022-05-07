#include <iostream>
#include <string>
#include <memory>
#include <boost/asio.hpp>

using namespace std;
using namespace boost;
using namespace asio;
using namespace asio::error;
int ii = 0;
class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:
    HttpConnection(asio::io_context& io) : socket_(io) {}

    void Start()
    {
        auto p = shared_from_this();
        asio::async_read_until(socket_, asio::dynamic_buffer(request_), "\r\n\r\n",
            [p, this](const error_code& err, size_t len) {
                if (err)
                {
                    cout << "recv err:" << err.message() << "\n";
                    return;
                }
                string first_line = request_.substr(0, request_.find("\r\n")); // should be like: GET / HTTP/1.0
                cout << first_line << "\n";
                // process with request
                // ...

                char str[59] = "HTTP/1.0 200 OK\r\n\r\n"
                    "<html>hello from http server</html>";
                str[55] = (ii % 10) + '0';
                str[56] = (ii/10 % 10) + '0';
                str[57] = (ii/100 % 10) + '0';
                str[58] = (ii/1000 % 10) + '0';
                ii++;
                asio::async_write(socket_, asio::buffer(str), [p, this](const error_code& err, size_t len) {
                    socket_.close();
                    });
            });
    }

    asio::ip::tcp::socket& Socket() { return socket_; }
private:
    asio::ip::tcp::socket socket_;
    string request_;
};

class HttpServer
{
public:
    HttpServer(asio::io_context& io, asio::ip::tcp::endpoint ep) : io_(io), acceptor_(io, ep) {}

    void Start()
    {
        auto p = std::make_shared<HttpConnection>(io_);
        acceptor_.async_accept(p->Socket(), [p, this](const error_code& err) {
            if (err)
            {
                cout << "accept err:" << err.message() << "\n";
                return;
            }
            p->Start();
            Start();
            });
    }
private:
    asio::io_context& io_;
    asio::ip::tcp::acceptor acceptor_;
};

int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        cout << "usage: httpsvr ip port\n";
        return 0;
    }

    asio::io_context io;
    asio::ip::tcp::endpoint ep(asio::ip::make_address(argv[1]), std::stoi(argv[2]));
    HttpServer hs(io, ep);
    hs.Start();
    io.run();
    return 0;
}
