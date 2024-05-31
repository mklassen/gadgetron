#include "Processable.h"


std::thread Gadgetron::Server::Connection::Processable::process_async(
    std::shared_ptr<Processable> processable,
    Core::GenericInputChannel input,
    Core::OutputChannel output,
    const ErrorHandler &error_handler
) {
    ErrorHandler nested_handler{error_handler, processable->name()};

    Processable::ProcessBarrier barrier;
    auto barrier_future = barrier.get_future();

    auto thread = nested_handler.run(
        [=, &barrier](auto input, auto output, auto error_handler) {
          processable->process(std::move(input), std::move(output), error_handler, std::move(barrier));
        },
        std::move(input),
        std::move(output),
        nested_handler
    );

    barrier_future.wait();

    return thread;
}
