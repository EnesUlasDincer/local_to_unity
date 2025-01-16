#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <chrono>
#include <cstring>

class PointCloudPublisher {
public:
    PointCloudPublisher(const std::string& ip_addr, const std::string& topic_name = "PointCloud")
        : topic_name_(topic_name), context_(1), pub_socket_(context_, zmq::socket_type::pub) {
        pub_socket_.bind("tcp://" + ip_addr + ":7721");
        int high_water_mark = 1;
        pub_socket_.setsockopt(ZMQ_SNDHWM, &high_water_mark, sizeof(high_water_mark));
    }

    void publish_bytes(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> msg(topic_name_.begin(), topic_name_.end());
        msg.push_back('|');
        msg.insert(msg.end(), data.begin(), data.end());

        zmq::message_t message(msg.data(), msg.size());
        pub_socket_.send(message, zmq::send_flags::none);
    }

private:
    std::string topic_name_;
    zmq::context_t context_;
    zmq::socket_t pub_socket_;
};

std::vector<uint8_t> generate_point() {
    std::vector<float> point(6);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    for (auto& val : point) {
        val = dis(gen);
    }

    std::vector<uint8_t> bytes(point.size() * sizeof(float));
    std::memcpy(bytes.data(), point.data(), bytes.size());
    return bytes;
}

std::vector<uint8_t> generate_point_cloud(int point_num = 10000) {
    std::vector<uint8_t> point_cloud;
    for (int i = 0; i < point_num; ++i) {
        auto point = generate_point();
        point_cloud.insert(point_cloud.end(), point.begin(), point.end());
    }
    return point_cloud;
}

int main() {
    PointCloudPublisher publisher("127.0.0.1");

    // print publisher
    

    try {
        while (true) {
            auto point_cloud = generate_point_cloud();
            publisher.publish_bytes(point_cloud);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
