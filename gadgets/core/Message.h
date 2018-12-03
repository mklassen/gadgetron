#pragma once

#include <memory>
#include <vector>

namespace Gadgetron {
    namespace Core {

        class Message {
        public:
            virtual ~Message() = 0;
        };


        template<class T>
        class TypedMessage : public Message {
        public:

            TypedMessage(const T& input) : data(std::make_unique<T>(input)) {

            }

            TypedMessage(T&& input) : data{std::make_unique<T>(input)} {

            }

            TypedMessage(std::unique_ptr<T> &&input_ptr) : data(std::move(input_ptr)) {

            }

            TypedMessage(TypedMessage &&other) : data(other.get_data()) {

            }

            TypedMessage(TypedMessage& other) = delete;

            std::unique_ptr<T> &&take_data() {
                return std::move(data);
            }

            virtual ~TypedMessage() {};


        protected:
            std::unique_ptr<T> data;

        };

        class MessageTuple : public Message {
        public:
            template<class ...ARGS>
            explicit MessageTuple(ARGS &&...  args) : Message(), messages_{std::move(make_message(std::move(args)))...} {

            }

            template<>
            explicit MessageTuple(std::vector<std::unique_ptr<Message>>&& message_vector) : messages_(std::move(message_vector)){

            }

            const std::vector<std::unique_ptr<Message>> &messages() const {
                return messages_;
            }

            std::vector<std::unique_ptr<Message>> &&take_messages() {
                return std::move(messages_);
            }

        private:

            template<class T> std::unique_ptr<Message> make_message(T&& input){
                return std::make_unique<TypedMessage<T>>(std::move(input));
            }

            std::vector<std::unique_ptr<Message>> messages_;




        };
    }
}