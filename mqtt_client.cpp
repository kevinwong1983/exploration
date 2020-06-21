#include "mqtt_client.h"

#include <iostream>

MqttClient::MqttClient(const std::string& client_id,
                       const std::string& server_address,
                       unsigned int server_port,
                       boost::asio::io_context &ioc)
        : mqtt_client_{MQTT_NS::make_async_client(ioc, server_address, server_port)} {
    mqtt_client_->set_client_id(client_id);
    mqtt_client_->set_clean_session(true);
    using packet_id_t = typename std::remove_reference_t<decltype(*mqtt_client_)>::packet_id_t;

    mqtt_client_->set_publish_handler(
            std::bind(&MqttClient::OnPublished, this, std::placeholders::_3, std::placeholders::_4));
    mqtt_client_->async_connect(
            std::bind(&MqttClient::OnConnected, this, std::placeholders::_1));
}

void MqttClient::Disconnect(){
    mqtt_client_->async_disconnect([](MQTT_NS::error_code ec){
        std::cout << "disconnected" << std::endl;
    });
}

void MqttClient::OnConnected(MQTT_NS::error_code ec) {
    if (!ec.failed()) {
        for (auto&& item : subscriptions_) {
            DoSubscribe(item.first, std::move(item.second));
        }
        subscriptions_.clear();

        for (auto&& item: publish_queque_) {
            mqtt_client_->async_publish(item.first, item.second, MQTT_NS::qos::at_least_once); // add retainment, expose qos on interface
        }
        publish_queque_.empty();
    }
}

bool MqttClient::OnPublished(MQTT_NS::buffer topic_name, MQTT_NS::buffer contents) {
    for (auto&& item : handlers_) {
        item(topic_name.to_string(), contents.to_string());
    }
    return true;
}

void MqttClient::Subscribe(Topic topic, MessageHandler handler) {
    subscriptions_.emplace_back(std::move(topic), std::move(handler));
}

void MqttClient::Publish(const Topic& topic, const Message& message) {
    if (mqtt_client_->connected()){
        mqtt_client_->async_publish(topic, message, MQTT_NS::qos::at_least_once);
    }
    else {
        publish_queque_.emplace_back(topic, message);
    }
}

std::function<void(const Topic&, const Message&)> MqttClient::CreateSingleLevelFilterHandler(
        std::string root_topic, std::string topic_path, MessageHandler handler) {
    return [root_topic = std::move(root_topic), topic_path = std::move(topic_path), handler = std::move(handler)]
            (const Topic& topic, const Message& message) {
        if (topic.find(root_topic) == 0 &&
            topic.rfind(topic_path) == topic.size() - topic_path.size() &&
            topic.size() > root_topic.size() + topic_path.size()) {
            const auto wildcard_value = topic.substr(root_topic.size(),
                                                     topic.size() - topic_path.size() - root_topic.size());
            if (wildcard_value.find('/') == std::string::npos) {
                handler(wildcard_value, message);
            }
        }
    };
}

std::function<void(const Topic&, const Message&)> MqttClient::CreateMultiLevelFilterHandler(
        std::string root_topic, MessageHandler handler) {
    return [root_topic = std::move(root_topic), handler = std::move(handler)]
            (const Topic& topic, const Message& message) {
        if (topic.find(root_topic) ==  0) {
            handler(topic.substr(root_topic.size()), message);
        }
    };
}

std::function<void(const Topic&, const Message&)> MqttClient::CreateTopicFilterHandler(
        std::string topic, MessageHandler handler) {
    return [root_topic = std::move(topic), handler = std::move(handler)]
            (const Topic& topic, const Message& message) {
        if (root_topic == topic) {
            handler("", message);
        }
    };
}

void MqttClient::DoSubscribe(const Topic& topic, MessageHandler handler) {
    MessageHandler handler_wrapper;
    const auto wildcard_iterator = std::find_if(std::begin(topic), std::end(topic),
                                                [](char ch) {return ch == '#' || ch == '+';});
    if (wildcard_iterator != std::end(topic)) {
        const auto wildcard_position = wildcard_iterator - std::begin(topic);
        auto root_topic = topic.substr(0, wildcard_position);
        if (*wildcard_iterator == '+') {
            auto topic_path = topic.substr(wildcard_position + 1);
            handler_wrapper = CreateSingleLevelFilterHandler(std::move(root_topic), std::move(topic_path), std::move(handler));
        } else {
            handler_wrapper = CreateMultiLevelFilterHandler(std::move(root_topic), std::move(handler));
        }
    } else {
        handler_wrapper = CreateTopicFilterHandler(topic, std::move(handler));
    }

    handlers_.emplace_back(std::move(handler_wrapper));

    if (mqtt_client_) {
        mqtt_client_->async_subscribe(topic, MQTT_NS::qos::at_least_once, [this, topic](MQTT_NS::error_code ec) {
            if (ec.failed()) {
                std::cout << "unable to subscribe to: " << topic << std::endl;
            }
        });
    }
}