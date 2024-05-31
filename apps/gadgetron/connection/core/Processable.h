#pragma once

#include "Channel.h"
#include "Context.h"
#include <memory>
#include <thread>
#include "connection/Core.h"
#include <future>

namespace Gadgetron::Server::Connection {

    class Processable {
    public:
        typedef std::promise<void> ProcessBarrier;

        virtual ~Processable() = default;

        virtual void process(
                Core::GenericInputChannel input,
                Core::OutputChannel output,
                ErrorHandler &error_handler
        ) = 0;

        virtual void process(
                Core::GenericInputChannel input,
                Core::OutputChannel output,
                ErrorHandler &error_handler,
                ProcessBarrier barrier)
        {
            barrier.set_value();
            process(std::move(input), std::move(output), error_handler);
        }

        virtual const std::string& name() = 0;

        static std::thread process_async(
            std::shared_ptr<Processable> processable,
            Core::GenericInputChannel input,
            Core::OutputChannel output,
            const ErrorHandler &errorHandler
        );
    };
}