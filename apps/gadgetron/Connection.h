#pragma once

#include <memory>
#include <iostream>

#include "Context.h"
#include <boost/asio.hpp>

namespace Gadgetron::Server::Connection {
    void handle(
            const Gadgetron::Core::StreamContext::Paths &paths,
            const Gadgetron::Core::StreamContext::Args &args,
            const std::string& storage,
            std::unique_ptr<boost::asio::ip::tcp::socket> psocket
    );
}
