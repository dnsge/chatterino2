//
// Adapted from https://www.boost.org/doc/libs/1_66_0/libs/beast/example/http/server/small/http_server_small.cpp
//

#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#define LOGIN_PORT_NUM 48920

namespace chatterino {

namespace {
    using TokenSignal = pajlada::Signals::Signal<QString>;

    namespace http = boost::beast::http;
    namespace beast = boost::beast;
    namespace net = boost::asio;
    using tcp = boost::asio::ip::tcp;
}  // namespace

class LoginServer : QObject
{
    Q_OBJECT
public:
    LoginServer(QObject *parent = nullptr)
        : QObject(parent)
    {
        moveToThread(&this->thread_);
        connect(&this->thread_, SIGNAL(started()), this, SLOT(run()));
    }

    static const QString portNumber()
    {
        return QString::number(LOGIN_PORT_NUM);
    }

    void start()
    {
        if (!this->running_)
        {
            this->running_ = true;
            this->thread_.start();
        }
    }

    void stop()
    {
        if (!this->running_)
            return;

        this->running_ = false;
        if (this->context_ != nullptr)
        {
            this->context_->stop();
        }
        if (this->acceptor_ != nullptr)
        {
            this->acceptor_->cancel();
            this->acceptor_->close();
        }
        this->thread_.quit();
        this->thread_.wait();
    }

    TokenSignal tokenSignal;

public slots:
    void run()
    {
        try
        {
            auto const address = net::ip::make_address("127.0.0.1");
            unsigned short port = static_cast<unsigned short>(LOGIN_PORT_NUM);

            net::io_context ioc{1};
            this->context_ = &ioc;

            tcp::acceptor acceptor{ioc, {address, port}};
            this->acceptor_ = &acceptor;

            tcp::socket socket{ioc};
            doAccept(acceptor, socket);

            ioc.run();
        }
        catch (std::exception const &e)
        {
            qDebug() << "LoginServer failed to start:" << e.what();
            return;
        }
    }

private:
    bool running_ = false;
    QThread thread_;

    net::io_context *context_ = nullptr;
    tcp::acceptor *acceptor_ = nullptr;

    void doAccept(tcp::acceptor &acceptor, tcp::socket &socket)
    {
        acceptor.async_accept(socket, [&](boost::beast::error_code ec) {
            if (!ec)
            {
                auto s = std::make_shared<HttpConnection>(std::move(socket));
                s->setCallback(&this->tokenSignal);
                s->start();
            }

            // accept next request
            doAccept(acceptor, socket);
        });
    }

    class HttpConnection : public std::enable_shared_from_this<HttpConnection>
    {
    public:
        HttpConnection(tcp::socket socket)
            : socket_(std::move(socket)){};

        void start()
        {
            this->readRequest();
            this->checkDeadline();
        }

        void setCallback(TokenSignal *signal)
        {
            this->signal_ = signal;
        }

    private:
        TokenSignal *signal_ = nullptr;

        tcp::socket socket_;
        beast::flat_buffer buffer_{8192};
        http::request<http::dynamic_body> request_;
        http::response<http::dynamic_body> response_;
        net::steady_timer deadline_{socket_.get_executor(),
                                    std::chrono::seconds(5)};

        void readRequest()
        {
            auto self = this->shared_from_this();

            http::async_read(
                socket_, buffer_, request_,
                [self](beast::error_code ec, std::size_t bytes_transferred) {
                    boost::ignore_unused(bytes_transferred);
                    if (!ec)
                        self->processRequest();
                });
        }

        void processRequest()
        {
            this->response_.version(request_.version());
            this->response_.keep_alive(false);

            switch (request_.method())
            {
                case http::verb::get:
                case http::verb::post:
                    this->createResponse();
                    break;
                case http::verb::options:
                    // cors preflight
                    this->response_.result(http::status::ok);
                    this->response_.set(
                        http::field::access_control_allow_origin, "*");
                    this->response_.set(
                        http::field::access_control_allow_headers,
                        "X-Token-Data");
                    break;
                default:
                    this->response_.result(http::status::bad_request);
                    this->response_.set(http::field::content_type,
                                        "text/plain");
                    boost::beast::ostream(this->response_.body())
                        << "Invalid request-method '"
                        << this->request_.method_string().to_string() << "'";
                    break;
            }

            this->writeResponse();
        }

        void createResponse()
        {
            if (this->request_.method() == http::verb::get)
            {
                if (this->request_.target() == "/login")
                {
                    this->response_.result(http::status::ok);
                    this->response_.set(http::field::content_type, "text/html");

                    QFile file(":/html/login.html");
                    file.open(QIODevice::ReadOnly);
                    boost::beast::ostream(this->response_.body())
                        << QString(file.readAll())
                               .replace("{{ port }}", LoginServer::portNumber())
                               .toStdString();
                    return;
                }
            }

            else if (this->request_.method() == http::verb::post)
            {
                if (this->request_.target() == "/token")
                {
                    this->response_.set(
                        http::field::access_control_allow_origin, "*");
                    this->response_.set(
                        http::field::access_control_allow_headers,
                        "X-Token-Data");

                    auto header = this->request_.base();
                    if (header.count("X-Token-Data") == 1)
                    {
                        this->response_.result(http::status::ok);
                        auto d = header.at("X-Token-Data");

                        std::string data{d.data(), d.size()};
                        if (this->signal_ != nullptr)
                        {
                            this->signal_->invoke(QString::fromStdString(data));
                        }
                    }
                    else
                    {
                        this->response_.result(http::status::bad_request);
                    }

                    return;
                }
            }

            this->response_.result(http::status::not_found);
            this->response_.set(http::field::content_type, "text/plain");
            boost::beast::ostream(this->response_.body())
                << "404 not found\r\n";
        }

        void writeResponse()
        {
            auto self = this->shared_from_this();

            this->response_.set(http::field::content_length,
                                this->response_.body().size());

            http::async_write(this->socket_, this->response_,
                              [self](boost::beast::error_code ec, std::size_t) {
                                  self->socket_.shutdown(
                                      tcp::socket::shutdown_send, ec);
                                  self->deadline_.cancel();
                              });
        }

        void checkDeadline()
        {
            auto self = shared_from_this();

            this->deadline_.async_wait([self](boost::beast::error_code ec) {
                if (!ec)
                {
                    // Close socket to cancel any outstanding operation.
                    self->socket_.close(ec);
                }
            });
        }
    };
};

}  // namespace chatterino
