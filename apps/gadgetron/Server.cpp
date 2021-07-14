
#include <boost/asio.hpp>

#include "../clients/gadgetron_ismrmrd_client/gadgetron_ismrmrd_client.cpp"

#include <Context.h>
#include "GadgetronSlotContainer.h"
#include "log.h"

#include "Gadget.h"
#include "../../network/messages/ConfigurationReader.h" // gotta find out how to include this file using submodule
#include "Connection.h"
#include "Gadget.h"
#include "Server.h"
#include "connection/SocketStreamBuf.h"
#include "system_info.h"
#include "LegacyACE.h"

using namespace boost::filesystem;
using namespace Gadgetron::Server;


Server::Server(
        const boost::program_options::variables_map &args
) : args(args) {}

void Server::serve() {

    Gadgetron::Core::Context::Paths paths{args["home"].as<path>(), args["dir"].as<path>()};
    GINFO_STREAM("Gadgetron home directory: " << paths.gadgetron_home);
    GINFO_STREAM("Gadgetron working directory: " << paths.working_folder);

#if(BOOST_VERSION >= 107000)
    boost::asio::io_context executor;
#else
    boost::asio::io_service executor;
#endif
    boost::asio::ip::tcp::endpoint local(Info::tcp_protocol(), args["port"].as<unsigned short>());
    boost::asio::ip::tcp::acceptor acceptor(executor, local);

    acceptor.set_option(boost::asio::socket_base::reuse_address(true));

    while(true) {
        auto socket = std::make_unique<boost::asio::ip::tcp::socket>(executor);
        acceptor.accept(*socket);

        GINFO_STREAM("Accepted connection from: " << socket->remote_endpoint().address());

        Connection::handle(paths, args, Gadgetron::Connection::stream_from_socket(std::move(socket)));
    }


    GadgetronSlotContainer<GadgetMessageReader> readers_;

    readers_.insert(GADGET_MESSAGE_CONFIG_FILE,
                    new GadgetNetworkMessageConfigFileReader());
    readers_.insert(GADGET_MESSAGE_CONFIG_SCRIPT,
                    new GadgetNetworkMessageScriptReader());

    readers_.insert(GADGET_MESSAGE_PARAMETER_SCRIPT,
                    new GadgetNetworkMessageScriptReader());
}

int svc() // not sure about this one
{
        while (true) {
        GadgetMessageIdentifier id;
        uint16_t msgType;
        ssize_t recv_cnt = 0;
        if ((recv_cnt = peer().recv_n (&msgType, sizeof(msgType))) <= 0) {
            GERROR("GadgetStreamController, unable to read message identifier\n");
            return -1;
        }
        id.id = ACE_NTOHS(msgType);

        if (id.id == GADGET_MESSAGE_CLOSE) {
            stream_.close(1); //Shutdown gadgets and wait for them
            GDEBUG("Stream closed\n");
            GDEBUG("Closing writer task\n");
            this->writer_task_.close(1);
            GDEBUG("Writer task closed\n");
            // Stream has closed we are finished, break out of loop
            return 0;
        }

        GadgetMessageReader* r = readers_.find(id.id);

        if (!r) {
            GERROR("Unrecognized Message ID received: %d\n", id.id);
            return GADGET_FAIL;
        }

        ACE_Message_Block* mb = r->read(&peer());

        if (!mb) {
            GERROR("GadgetMessageReader returned null pointer\n");
            return GADGET_FAIL;
        }

        //We need to handle some special cases to make sure that we can get a stream set up.
        if (id.id == GADGET_MESSAGE_CONFIG_FILE) {
            Gadgetron::GadgetContainerMessage<GadgetMessageConfigurationFile>* cfgm =
                AsContainerMessage<GadgetMessageConfigurationFile>(mb);

            if (!cfgm) {
                GERROR("Failed to cast message block to configuration file\n");
                mb->release();
                return GADGET_FAIL;
            } else {
                if (this->configure_from_file(std::string(cfgm->getObjectPtr()->configuration_file)) != GADGET_OK) {
                    GERROR("GadgetStream configuration failed\n");
                    mb->release();
                    return GADGET_FAIL;
                } else {
                    mb->release();
                    continue;
                }
            }
        } else if (id.id == GADGET_MESSAGE_CONFIG_SCRIPT) {
            std::string xml_config(mb->rd_ptr(), mb->length());
            std::stringstream stream(xml_config, std::ios::in);
            if (this->configure(stream) != GADGET_OK) {
                GERROR("GadgetStream configuration failed\n");
                mb->release();
                return GADGET_FAIL;
            } else {
                mb->release();
                continue;
            }
        }

        ACE_Time_Value wait = ACE_OS::gettimeofday() + ACE_Time_Value(0,10000); //10ms from now
        if (stream_.put(mb) == -1) {
            GERROR("Failed to put stuff on stream, too long wait, %d\n",  ACE_OS::last_error () ==  EWOULDBLOCK);
            mb->release();
            return GADGET_FAIL;
        }
    }
    return GADGET_OK;
}