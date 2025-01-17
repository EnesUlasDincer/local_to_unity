#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <chrono>
#include <cstring>

struct Point {
    float x, y, z;
    float r, g, b;
};

class PointCloudProcessor {
public:
    static void processPointClouds(const std::vector<Point>& pc1, const std::vector<Point>& pc2);

    static std::vector<Point> deserializePointCloud(const void* data, size_t byteArraySize) {
        constexpr size_t FLOATS_PER_POINT = 6;
        constexpr size_t POINT_SIZE = FLOATS_PER_POINT * sizeof(float);
        size_t pointCount = byteArraySize / POINT_SIZE;

        const float* floatArray = static_cast<const float*>(data);
        std::vector<Point> cloud;
        cloud.reserve(pointCount); // Avoids multiple reallocations

        // Corrected lambda to take a float pointer properly
        for (size_t i = 0; i < pointCount; ++i) {
            cloud.emplace_back(Point{
                floatArray[i * FLOATS_PER_POINT]     / 1000, 
                floatArray[i * FLOATS_PER_POINT + 1] / 1000,
                floatArray[i * FLOATS_PER_POINT + 2] / 1000,
                floatArray[i * FLOATS_PER_POINT + 3] / 255,
                floatArray[i * FLOATS_PER_POINT + 4] / 255,
                floatArray[i * FLOATS_PER_POINT + 5] / 255
            });
        }

        return cloud;
    }

    static std::pair<std::unique_ptr<float[]>, size_t> serializePointCloud(const std::vector<Point>& cloud) {
        constexpr size_t FLOATS_PER_POINT = 6;
        size_t pointCount = cloud.size();
        size_t byteArraySize = pointCount * FLOATS_PER_POINT * sizeof(float);

        std::unique_ptr<float[]> serializedData(new float[pointCount * FLOATS_PER_POINT]);

        // Direct memory copy is not valid, use a loop instead
        for (size_t i = 0; i < pointCount; ++i) {
            serializedData[i * FLOATS_PER_POINT]     = cloud[i].x;
            serializedData[i * FLOATS_PER_POINT + 1] = cloud[i].y;
            serializedData[i * FLOATS_PER_POINT + 2] = cloud[i].z;
            serializedData[i * FLOATS_PER_POINT + 3] = cloud[i].r;
            serializedData[i * FLOATS_PER_POINT + 4] = cloud[i].g;
            serializedData[i * FLOATS_PER_POINT + 5] = cloud[i].b;
        }

        return { std::move(serializedData), byteArraySize };
    }

};


void PointCloudProcessor::processPointClouds(const std::vector<Point>& pc1, const std::vector<Point>& pc2) {
    std::cout << "Processing point clouds..." << std::endl;
    std::cout << "Point Cloud 1 has " << pc1.size() << " points." << std::endl;
    std::cout << "Point Cloud 2 has " << pc2.size() << " points." << std::endl;
}

class PointCloudPublisher {
public:
    PointCloudPublisher(const std::string& ip_addr, const std::string& topic_name = "PointCloud");
    void publishBytes(const float* data, size_t byteArraySize);
private:
    std::string topic_name_;
    zmq::context_t context_;
    zmq::socket_t pub_socket_;
};

PointCloudPublisher::PointCloudPublisher(const std::string& ip_addr, const std::string& topic_name)
    : topic_name_(topic_name), context_(1), pub_socket_(context_, zmq::socket_type::pub) {
    pub_socket_.bind("tcp://" + ip_addr + ":7721");
    int high_water_mark = 1;
    pub_socket_.setsockopt(ZMQ_SNDHWM, &high_water_mark, sizeof(high_water_mark));
    // pub_socket_.setsockopt(ZMQ_SNDHWM, &high_water_mark, sizeof(high_water_mark));
}

void PointCloudPublisher::publishBytes(const float* data, size_t byteArraySize) {
    std::vector<uint8_t> msg(topic_name_.begin(), topic_name_.end());
    msg.push_back('|');
    msg.insert(msg.end(), reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + byteArraySize);
    zmq::message_t message(msg.data(), msg.size());
    pub_socket_.send(message, zmq::send_flags::none);
    std::cout << "Sent processed point cloud with topic: " << topic_name_ << " (" << byteArraySize << " bytes)." << std::endl;
}

class ZMQPointCloudServer {
public:
    ZMQPointCloudServer(PointCloudPublisher& publisher);
    void run();
private:
    zmq::context_t context;
    zmq::socket_t receiver1;
    zmq::socket_t receiver2;
    PointCloudPublisher& publisher_;
};

ZMQPointCloudServer::ZMQPointCloudServer(PointCloudPublisher& publisher)
    : context(1), receiver1(context, ZMQ_PULL), receiver2(context, ZMQ_PULL), publisher_(publisher) {
    receiver1.bind("tcp://*:5552");
    receiver2.bind("tcp://*:5553");
    std::cout << "Server initialized and sockets bound." << std::endl;
}

// publishes what is avaliable in the receiver1 and receiver2
// void ZMQPointCloudServer::run() {
//     zmq::pollitem_t items[] = {
//         { static_cast<void*>(receiver1), 0, ZMQ_POLLIN, 0 },
//         { static_cast<void*>(receiver2), 0, ZMQ_POLLIN, 0 }
//     };

//     while (true) {
//         zmq::poll(items, 2, std::chrono::milliseconds(10)); // Poll sockets (10ms timeout)

//         std::vector<Point> combinedCloud;

//         for (int i = 0; i < 2; ++i) {
//             if (items[i].revents & ZMQ_POLLIN) {
//                 zmq::message_t message;
//                 if (auto result = (i == 0 ? receiver1 : receiver2).recv(message, zmq::recv_flags::none)) {
//                     auto pc = PointCloudProcessor::deserializePointCloud(message.data(), message.size());
//                     combinedCloud.insert(combinedCloud.end(), pc.begin(), pc.end());
//                 } else {
//                     std::cerr << "Failed to receive data from Client " << (i + 1) << std::endl;
//                 }
//             }
//         }

//         if (!combinedCloud.empty()) {
//             auto [serializedData, byteArraySize] = PointCloudProcessor::serializePointCloud(combinedCloud);
//             publisher_.publishBytes(serializedData.get(), byteArraySize);
//         }
//     }
// }

void ZMQPointCloudServer::run() {
    zmq::pollitem_t items[] = {
        { static_cast<void*>(receiver1), 0, ZMQ_POLLIN, 0 },
        { static_cast<void*>(receiver2), 0, ZMQ_POLLIN, 0 }
    };

    while (true) {
        std::vector<Point> combinedCloud;
        bool receivedFrom[2] = {false, false}; // Track which clients sent data

        while (!(receivedFrom[0] && receivedFrom[1])) { // Wait until both cameras send data
            zmq::poll(items, 2, std::chrono::milliseconds(10));

            for (int i = 0; i < 2; ++i) {
                if (items[i].revents & ZMQ_POLLIN) {
                    zmq::message_t message;
                    if (auto result = (i == 0 ? receiver1 : receiver2).recv(message, zmq::recv_flags::none)) {
                        auto pc = PointCloudProcessor::deserializePointCloud(message.data(), message.size());
                        combinedCloud.insert(combinedCloud.end(), pc.begin(), pc.end());
                        receivedFrom[i] = true; // Mark this camera as received
                    } else {
                        std::cerr << "Failed to receive data from Client " << (i + 1) << std::endl;
                    }
                }
            }
        }

        // Only publish when both clients have sent data
        if (!combinedCloud.empty()) {
            auto [serializedData, byteArraySize] = PointCloudProcessor::serializePointCloud(combinedCloud);
            publisher_.publishBytes(serializedData.get(), byteArraySize);
        }
    }
}


int main() {
    PointCloudPublisher publisher("127.0.0.1");
    ZMQPointCloudServer server(publisher);
    server.run();
    return 0;
}
