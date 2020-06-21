#pragma once

#include <mqtt_client_cpp.hpp>
#include <boost/variant.hpp>

using Topic = std::string;
using WildcardValue = std::string;
using Message = std::string;

class MqttClient {
public:
    using Client = std::shared_ptr<MQTT_NS::callable_overlay<MQTT_NS::async_client<MQTT_NS::tcp_endpoint<as::ip::tcp::socket, as::io_context::strand>>>>;
    using MessageHandler = std::function<void(const WildcardValue&, const Message&)>;

    MqttClient(const std::string& client_id,
               const std::string& server_address,
               unsigned int server_port,
               boost::asio::io_context &ioc);

    void Disconnect();
    void Subscribe(Topic topic, MessageHandler handler);
    void Publish(const Topic& topic, const Message& message);

protected:
    MqttClient() = default;
    void DoSubscribe(const Topic& topic, MessageHandler handler);

    void OnConnected(MQTT_NS::error_code ec);
    bool OnPublished(MQTT_NS::buffer topic_name,
                     MQTT_NS::buffer contents);

    std::function<void(const Topic&, const Message&)> CreateSingleLevelFilterHandler(
            std::string root_topic, std::string topic_path, MessageHandler handler);
    std::function<void(const Topic&, const Message&)> CreateMultiLevelFilterHandler(
            std::string root_topic, MessageHandler handler);
    std::function<void(const Topic&, const Message&)> CreateTopicFilterHandler(
            std::string topic, MessageHandler handler);

private:
    Client mqtt_client_;
    std::vector<std::function<void(const Topic&, const Message&)>> handlers_;
    std::vector<std::pair<Topic, MessageHandler>> subscriptions_;
    std::vector<std::pair<Topic, Message>> publish_queque_; // how long can this grow?
};