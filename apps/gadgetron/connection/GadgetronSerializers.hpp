#ifndef GADGETRONSERIALIZERS_HPP
#define GADGETRONSERIALIZERS_HPP
#include <ismrmrd/meta.h>
#include "connection/Serializer.hpp"
#include "connection/Serializers.hpp"
#include "connection/MessageID.hpp"
#include "writers/ImageWriter.h"
#include "writers/AcquisitionWriter.h"
#include "writers/WaveformWriter.h"
#include "MessageID.h"

namespace Gadgetron::Connection {
class ConfigFileSerializer : public ::Connection::Serializers::ConfigFileSerializer{
  public:

    ConfigFileSerializer() { m_id = Core::MessageID::FILENAME; }

    void serialize(std::ostream& ostrm, connection::any& obj) const override {
        ConfigFile &rConfigFilename = connection::any_cast<ConfigFile &>(obj);

        const std::string& config_filename = rConfigFilename.get_str(); //is this copy necessary?

        std::array<char,1024> data;
        std::copy(config_filename.begin(), config_filename.end(), data.data());
        data[config_filename.size()] = '\0';

        Core::IO::write<std::array<char,1024>>(ostrm, data);
    }
};

class ConfigScriptSerializer : public ::Connection::Serializers::ConfigScriptSerializer {
  public:

    ConfigScriptSerializer() { m_id = Core::MessageID::CONFIG; }

    void serialize(std::ostream& ostrm, connection::any& obj) const override {
        ConfigScript &rConfigScript = connection::any_cast<ConfigScript &>(obj);

        const std::string& config_script = rConfigScript.get_str();
        auto n = static_cast<uint32_t>(config_script.size());
        Core::IO::write<uint32_t>(ostrm, n);
        ostrm.write(config_script.c_str(), n);
    }
};

class HeaderSerializer : public ::Connection::Serializers::HeaderSerializer{
  public:

    HeaderSerializer() { m_id = Core::MessageID::HEADER; }

    void serialize(std::ostream& ostrm, connection::any& obj) const override {
        ISMRMRD::IsmrmrdHeader& rHeader = connection::any_cast<ISMRMRD::IsmrmrdHeader&>(obj);
        std::stringstream ss;
        ISMRMRD::serialize(rHeader,ss);
        Core::IO::write<uint32_t>(ostrm, static_cast<uint32_t>(ss.tellp()));
        ostrm << ss.str();
    }
};

class ImageSerializer : public ::Connection::Serializers::ImageSerializer{
  public:
    ImageSerializer() { m_id = Core::MessageID::GADGET_MESSAGE_ISMRMRD_IMAGE; }

    void serialize(std::ostream &ostrm, connection::any &obj) const override {
        if (obj.type() == typeid(ISMRMRD::Image<uint16_t>)) {
            serialize<uint16_t>(ostrm, obj);
        } else if (obj.type() == typeid(ISMRMRD::Image<int16_t>)) {
            serialize<int16_t>(ostrm, obj);
        } else if (obj.type() == typeid(ISMRMRD::Image<uint32_t>)) {
            serialize<uint32_t>(ostrm, obj);
        } else if (obj.type() == typeid(ISMRMRD::Image<int32_t>)) {
            serialize<int32_t>(ostrm, obj);
        } else if (obj.type() == typeid(ISMRMRD::Image<float>)) {
            serialize<float>(ostrm, obj);
        } else if (obj.type() == typeid(ISMRMRD::Image<double>)) {
            serialize<double>(ostrm, obj);
        } else if (obj.type() == typeid(ISMRMRD::Image<std::complex<float>>)) {
            serialize<std::complex<float>>(ostrm, obj);
        } else if (obj.type() == typeid(ISMRMRD::Image<std::complex<double>>)) {
            serialize<std::complex<double>>(ostrm, obj);
        }
    }

  private:
    template<typename T>
    void serialize(std::ostream &ostrm, connection::any &obj) const{
        ISMRMRD::Image<T>& rImg = connection::any_cast<ISMRMRD::Image<T>&>(obj);
        auto hdr = rImg.getHead();

        auto image_data = hoNDArray<T>(hdr.matrix_size[0], hdr.matrix_size[1], hdr.matrix_size[2], hdr.channels, rImg.getDataPtr());

        Core::optional<ISMRMRD::MetaContainer> meta;
        if (rImg.getAttributeStringLength()>0){
            meta = ISMRMRD::MetaContainer();
            ISMRMRD::deserialize(rImg.getAttributeString(), *meta);
        }

        Core::Writers::ImageWriter writer;
        writer.write(ostrm, Core::Message(hdr,std::move(image_data),std::move(meta)));
    }
};

class AcquisitionSerializer : public ::Connection::Serializers::AcquisitionSerializer{
  public:
    AcquisitionSerializer() { m_id = Core::MessageID::GADGET_MESSAGE_ISMRMRD_ACQUISITION; }

    void serialize(std::ostream &ostrm, connection::any &obj) const {
        ISMRMRD::Acquisition &rAcq = connection::any_cast<ISMRMRD::Acquisition &>(obj);

        auto hdr = rAcq.getHead();

        auto data = hoNDArray<std::complex<float>>(hdr.number_of_samples, hdr.active_channels, rAcq.getDataPtr());

        Core::optional<hoNDArray<float>> trajectory;
        if (hdr.trajectory_dimensions) {
            trajectory = hoNDArray<float>(hdr.trajectory_dimensions, hdr.number_of_samples, rAcq.getTrajPtr());
        }

        Core::Writers::AcquisitionWriter writer;
        writer.write(ostrm, Core::Message(hdr, std::move(data), std::move(trajectory)));
    }
};

class WaveformSerializer : public ::Connection::Serializers::WaveformSerializer{
  public:
    WaveformSerializer() { m_id = Core::MessageID::GADGET_MESSAGE_ISMRMRD_WAVEFORM; }

    void serialize(std::ostream &ostrm, connection::any &obj) const {
        ISMRMRD::Waveform &rWvfrm = connection::any_cast<ISMRMRD::Waveform &>(obj);
        ISMRMRD::ISMRMRD_WaveformHeader& hdr = rWvfrm.head;
        auto data = hoNDArray<uint32_t>(hdr.number_of_samples, hdr.channels, rWvfrm.data);
        Core::Writers::WaveformWriter writer;
        writer.write(ostrm, Core::Message(hdr, std::move(data)));
    }
};
} //namespace Gadgetron::Connection
#endif // GADGETRONSERIALIZERS_HPP
