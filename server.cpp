#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;

int main() 
{
    io_service service;
    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 2001));

    std::cout << "Waiting for connection..." << std::endl;

    ip::tcp::socket sock(service);
    acceptor.accept(sock);

    std::cout << "Client connected." << std::endl;

    try {
        for (;;) {
            char data[1024];
            size_t length = sock.read_some(buffer(data));

            if (length > 0) {
                std::cout << "Received data from client: " << std::string(data, length) << std::endl;
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << std::endl;
    }

    return 0;
}
