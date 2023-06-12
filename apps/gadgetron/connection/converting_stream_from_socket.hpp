#ifndef CONVERTING_STREAM_FROM_SOCKET_HPP
#define CONVERTING_STREAM_FROM_SOCKET_HPP
#include <memory>
#include "connection/ConvertingIOStream.hpp"
#include "connection/Serializers.hpp"
#include "connection/Deserializers.hpp"
#include "connection/GadgetronSerializers.hpp"
#include "connection/GadgetronDeserializers.hpp"


namespace Gadgetron::Connection {
std::unique_ptr<std::iostream> converting_stream_from_socket(std::unique_ptr<boost::asio::ip::tcp::socket> socket,
                                                             boost::asio::io_context& ioc) {
    // connection project serializers & deserializers
    auto pConfigFileSer = std::make_shared<::Connection::Serializers::ConfigFileSerializer>();
    auto pConfigScriptSer = std::make_shared<::Connection::Serializers::ConfigScriptSerializer>();
    auto pHeaderSer = std::make_shared<::Connection::Serializers::HeaderSerializer>();
    auto pImgSer = std::make_shared<::Connection::Serializers::ImageSerializer>();
    auto pAcqSer = std::make_shared<::Connection::Serializers::AcquisitionSerializer>();
    auto pWvfrmSer = std::make_shared<::Connection::Serializers::WaveformSerializer>();
    
    auto pConfigFileDes = std::make_shared<::Connection::Deserializers::ConfigFileDeserializer>();
    auto pConfigScriptDes = std::make_shared<::Connection::Deserializers::ConfigScriptDeserializer>();
    auto pHeaderDes = std::make_shared<::Connection::Deserializers::HeaderDeserializer>();
    auto pImgDes = std::make_shared<::Connection::Deserializers::ImageDeserializer>();
    auto pAcqDes = std::make_shared<::Connection::Deserializers::AcquisitionDeserializer>();
    auto pWvfrmDes = std::make_shared<::Connection::Deserializers::WaveformDeserializer>();
    
    // gadgetron project serializers & deserializers
    auto pGConfigFileSer = std::make_shared<ConfigFileSerializer>();
    auto pGConfigScriptSer = std::make_shared<ConfigScriptSerializer>();
    auto pGHeaderSer = std::make_shared<HeaderSerializer>();
    auto pGImgSer = std::make_shared<ImageSerializer>();
    auto pGAcqSer = std::make_shared<AcquisitionSerializer>();
    auto pGWvfrmSer = std::make_shared<WaveformSerializer>();

    auto pGImgDes = std::make_shared<ImageDeserializer>();
    auto pGAcqDes = std::make_shared<AcquisitionDeserializer>();
    auto pGWvfrmDes = std::make_shared<WaveformDeserializer>();

    
    std::list<std::shared_ptr<::Connection::Serializers::Serializer>> net_serializers = {
        pConfigFileSer, pConfigScriptSer, pHeaderSer, pImgSer, pAcqSer, pWvfrmSer
    };
    std::list<std::shared_ptr<::Connection::Deserializers::Deserializer>> net_deserializers = {
        pConfigFileDes, pConfigScriptDes, pHeaderDes, pImgDes, pAcqDes, pWvfrmDes
    };
    std::list<std::shared_ptr<::Connection::Serializers::Serializer>> gad_serializers = {
        pGConfigFileSer, pGConfigScriptSer, pGHeaderSer, pGImgSer, pGAcqSer, pGWvfrmSer
    };
    std::list<std::shared_ptr<::Connection::Deserializers::Deserializer>> gad_deserializers = {
        pGImgDes, pGAcqDes, pGWvfrmDes
    };

    return std::make_unique<ConvertingIOStream>(ioc, std::move(socket), gad_serializers, gad_deserializers,
                                                net_serializers, net_deserializers);
}
} // namespace Gadgetron::Connection

#endif // CONVERTING_STREAM_FROM_SOCKET_HPP
