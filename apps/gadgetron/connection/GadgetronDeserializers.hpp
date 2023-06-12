#ifndef GADGETRON_GADGETRONDESERIALIZERS_HPP
#define GADGETRON_GADGETRONDESERIALIZERS_HPP
#include "Message.h"
#include "connection/Deserializer.hpp"
#include "connection/WrappedIsmrmrd.hpp"
#include "readers/AcquisitionReader.h"
#include "readers/ImageReader.h"
#include "readers/WaveformReader.h"

namespace Gadgetron::Connection {
class ImageDeserializer : public ::Connection::Deserializers::Deserializer {
  public:
    ImageDeserializer() { m_id = Core::MessageID::GADGET_MESSAGE_ISMRMRD_IMAGE; }

    connection::any deserialize(std::istream& istrm) const override {
        Core::Readers::ImageReader reader;
        Core::Message msg = reader.read(istrm);
        if (Core::convertible_to<Core::Image<uint16_t>>(msg)) {
            return deserialize<uint16_t>(istrm, msg);
        } else if (Core::convertible_to<Core::Image<int16_t>>(msg)) {
            return deserialize<int16_t>(istrm, msg);
        } else if (Core::convertible_to<Core::Image<uint32_t>>(msg)) {
            return deserialize<uint32_t>(istrm, msg);
        } else if (Core::convertible_to<Core::Image<int32_t>>(msg)) {
            return deserialize<int32_t>(istrm, msg);
        } else if (Core::convertible_to<Core::Image<float>>(msg)) {
            return deserialize<float>(istrm, msg);
        } else if (Core::convertible_to<Core::Image<double>>(msg)) {
            return deserialize<double>(istrm, msg);
        } else if (Core::convertible_to<Core::Image<std::complex<float>>>(msg)) {
            return deserialize<std::complex<float>>(istrm, msg);
        } else if (Core::convertible_to<Core::Image<std::complex<double>>>(msg)) {
            return deserialize<std::complex<double>>(istrm, msg);
        } else {
            throw std::runtime_error("Trying to deserialize message with unrecognized Image type");
        }
    }

  private:
    template <typename T> connection::any deserialize(std::istream& istrm, Core::Message& msg) const {
        auto [hdr, data, meta] = Core::force_unpack<Core::Image<T>>(std::move(msg));
        ISMRMRD::ISMRMRD_Image img_struct;
        img_struct.head = hdr;
        img_struct.data = data.get_data_ptr();
        if (meta) {
            std::stringstream ss;
            ISMRMRD::serialize(*meta, ss);
            auto meta_str = ss.str();
            size_t length = meta_str.size();
            img_struct.head.attribute_string_len = static_cast<uint32_t>(length);
            img_struct.attribute_string =
                new char[length + 1]; // this won't be deleted by WrappedImage going out of scope
            strncpy(img_struct.attribute_string, meta_str.c_str(), length + 1);
        }
        else {
            img_struct.attribute_string = NULL;
        }
        // the data object will drop out of scope after this function returns and if it deletes the data then the image
        // class that was initialized with its data will have dangling pointers.  Instead, we let the image class
        // take care of clean up.
        data.delete_data_on_destruct(false);
        WrappedImage<T> img(img_struct, true);

        // The following is bad, don't do it
        //ISMRMRD shared library will try to reallocate memory for a new string,
        // but gadgetron owns the memory of the ISMRMRD_Image struct and the string!
        //img.setAttributeString(ss.str());
        return connection::any(std::move(img));
    }
};

class AcquisitionDeserializer : public ::Connection::Deserializers::Deserializer {
  public:
    AcquisitionDeserializer() { m_id = Core::MessageID::GADGET_MESSAGE_ISMRMRD_ACQUISITION; }

    connection::any deserialize(std::istream& istrm) const override {
        Core::Readers::AcquisitionReader reader;
        Core::Message msg = reader.read(istrm);
        auto [hdr, data, traj] = Core::force_unpack<Core::Acquisition>(std::move(msg));
        ISMRMRD::ISMRMRD_Acquisition acq_struct;
        acq_struct.head = hdr;

        acq_struct.data = data.get_data_ptr();
        if (traj)
            acq_struct.traj = traj->get_data_ptr();
        else
            acq_struct.traj = NULL;
        // the data object will drop out of scope after this function returns and if it deletes the data then the
        // acquisition class that was initialized with its data will have dangling pointers.  Instead, we let the
        // acquisition class take care of clean up.
        data.delete_data_on_destruct(false);
        if (traj)
            traj->delete_data_on_destruct(false);
        WrappedAcquisition acq(acq_struct, true);
        return connection::any(std::move(acq));
    }
};

class WaveformDeserializer : public ::Connection::Deserializers::Deserializer {
  public:

    WaveformDeserializer() { m_id = Core::MessageID::GADGET_MESSAGE_ISMRMRD_WAVEFORM; }

    connection::any deserialize(std::istream& istrm) const override {
        Core::Readers::WaveformReader reader;
        Core::Message msg = reader.read(istrm);
        auto [hdr, data] = Core::force_unpack<Core::Waveform>(std::move(msg));
        ISMRMRD::Waveform wvfrm;
        wvfrm.head = hdr;
        wvfrm.data = data.get_data_ptr();

        // the data object will drop out of scope after this function returns and if it deletes the data then the
        // waveform class that was initialized with its data will have dangling pointers.  Instead, we let the
        // waveform class take care of clean up.
        data.delete_data_on_destruct(false);
        return connection::any(std::move(wvfrm));
    }
};

} // namespace Gadgetron::Connection

#endif // GADGETRON_GADGETRONDESERIALIZERS_HPP
