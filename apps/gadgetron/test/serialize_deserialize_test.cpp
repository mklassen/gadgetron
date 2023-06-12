#include <gtest/gtest.h>
#include "connection/GadgetronSerializers.hpp"
#include "connection/GadgetronDeserializers.hpp"
#include "io/primitives.h"

using namespace Gadgetron;
using namespace Gadgetron::Core;

TEST(SerDesTest, ConfigFile){
    std::string filename = "filename.config";
    std::any msg((ConfigFile(filename)));
    Gadgetron::Connection::ConfigFileSerializer ser;
    std::stringstream ss;
    ser.serialize(ss, msg);

    // ConfigReferenceHandler in gadgetron/apps/gadgetron/connection/ConfigConnection.cpp
    auto buffer = IO::read<std::array<char,1024>>(ss);
    std::string recv_string(buffer.data());

    EXPECT_EQ(recv_string, filename);
}

TEST(SerDesTest, ConfigScript){
    std::string script_data = "some script data";
    std::any msg((ConfigScript(script_data)));
    Gadgetron::Connection::ConfigScriptSerializer ser;
    std::stringstream ss;
    ser.serialize(ss, msg);

    // ConfigStringHandler in gadgetron/apps/gadgetron/connection/ConfigConnection.cpp
    auto recv_string = IO::read_string_from_stream<uint32_t>(ss);

    EXPECT_EQ(recv_string, script_data);
}

TEST(SerDesTest, Header){
    ISMRMRD::IsmrmrdHeader h;
    // required header components
    h.encoding.push_back(ISMRMRD::Encoding());
    ISMRMRD::MeasurementInformation mi;
    mi.patientPosition = "required";
    h.measurementInformation = mi;

    std::any msg(h);
    Gadgetron::Connection::HeaderSerializer ser;
    std::stringstream ss;
    ser.serialize(ss, msg);

    // HeaderHandler in gadgetron/apps/gadgetron/connection/HeaderConnection.cpp
    auto recv_string = IO::read_string_from_stream<uint32_t>(ss);
    ISMRMRD::IsmrmrdHeader recv_header{};
    ISMRMRD::deserialize(recv_string.c_str(), recv_header);

    EXPECT_EQ(recv_header, h);
}

TEST(SerDesTest, Image){
    ISMRMRD::ISMRMRD_ImageHeader hdr_base;
    ISMRMRD::ismrmrd_init_image_header(&hdr_base);
    uint16_t matrix_size[] = {5, 5, 1};
    uint16_t nchannels = 1;
    // ISMRMRD has short but gadgetron reader doesn't support short, use anything but short!!
    //uint16_t dtype = ISMRMRD::ISMRMRD_SHORT;
    uint16_t dtype = ISMRMRD::ISMRMRD_USHORT;
    hdr_base.matrix_size[0] = matrix_size[0];
    hdr_base.matrix_size[1] = matrix_size[1];
    hdr_base.matrix_size[2] = matrix_size[2];
    hdr_base.channels = nchannels;
    hdr_base.data_type = dtype;
    hdr_base.measurement_uid = 1234;
    ISMRMRD::ImageHeader hdr = ISMRMRD::ImageHeader(hdr_base);

    ISMRMRD::MetaContainer meta;
    meta.append("Map Type", "B1");
    std::stringstream ss;
    ISMRMRD::serialize(meta, ss);
    std::string attribute_string = ss.str();

    ISMRMRD::Image<uint16_t> test_img;
    test_img.setHead(hdr);
    test_img.setAttributeString(attribute_string);


    ss.str("");
    Gadgetron::Connection::ImageSerializer ser;
    Gadgetron::Connection::ImageDeserializer des;

    auto msg = connection::any(test_img);
    ser.serialize(ss, msg);

    ASSERT_EQ(Core::IO::read<uint16_t>(ss), Core::MessageID::GADGET_MESSAGE_ISMRMRD_IMAGE);

    auto recv_msg = des.deserialize(ss);
    auto recv_img = connection::any_cast<WrappedImage<uint16_t>>(recv_msg);

    ASSERT_EQ(test_img.getHead(), recv_img.getHead());
    auto attr1 = static_cast<std::string>(test_img.getAttributeString());
    auto attr2 = static_cast<std::string>(recv_img.getAttributeString());
    ASSERT_EQ(attr1,attr2);
    EXPECT_TRUE( 0 == std::memcmp( test_img.getDataPtr(), recv_img.getDataPtr(), test_img.getDataSize() ) );
}

TEST(SerDesTest, Acquisition){
    ISMRMRD::Acquisition test_acq(100,1,1);
    ISMRMRD::AcquisitionHeader hdr = test_acq.getHead();
    hdr.measurement_uid=1;
    test_acq.setHead(hdr);

    std::stringstream ss;

    Gadgetron::Connection::AcquisitionSerializer ser;
    Gadgetron::Connection::AcquisitionDeserializer des;

    auto msg = connection::any(test_acq);
    ser.serialize(ss, msg);

    ASSERT_EQ(Core::IO::read<uint16_t>(ss), Core::MessageID::GADGET_MESSAGE_ISMRMRD_ACQUISITION);

    auto recv_msg = des.deserialize(ss);
    auto recv_acq = connection::any_cast<WrappedAcquisition>(recv_msg);

    ASSERT_EQ(test_acq.getHead(), recv_acq.getHead());
    EXPECT_TRUE( 0 == std::memcmp( test_acq.getDataPtr(), recv_acq.getDataPtr(), test_acq.getDataSize() ) );
    EXPECT_TRUE( 0 == std::memcmp( test_acq.getTrajPtr(), recv_acq.getTrajPtr(), test_acq.getTrajSize() ) );
}

TEST(SerDesTest, Waveform){
    ISMRMRD::Waveform test_wvfrm(100,1);
    test_wvfrm.head.measurement_uid = 1;
    std::stringstream ss;

    Gadgetron::Connection::WaveformSerializer ser;
    Gadgetron::Connection::WaveformDeserializer des;

    auto msg = connection::any(test_wvfrm);
    ser.serialize(ss, msg);

    ASSERT_EQ(Core::IO::read<uint16_t>(ss), Core::MessageID::GADGET_MESSAGE_ISMRMRD_WAVEFORM);

    auto recv_msg = des.deserialize(ss);
    auto recv_wvfrm = connection::any_cast<ISMRMRD::Waveform>(recv_msg);

    ASSERT_EQ(recv_wvfrm, test_wvfrm);
}