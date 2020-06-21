#include "boost/variant.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>

#include "mqtt_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

TEST(mqtt, start_mqtt_broker1) {
    boost::asio::io_context ioc;

    std::promise<std::string> done;

    pid_t processId;
    if ((processId = fork()) == 0) {
        char app[] = "/usr/local/sbin/mosquitto";
        char arg0[] = "/usr/local/sbin/mosquitto";
        char arg1[] = "-c";
        char arg2[] = "/usr/local/etc/mosquitto/mosquitto.conf";
        char * const argv[] = { arg0, arg1, arg2 , NULL };
        std::cout<< "done-a " <<std::endl;
        execv (app, argv);
    }

//    auto t1 = std::thread([&done](){
//
//    }

    sleep(1);
    std::cout<< "done-b " <<std::endl;
    EXPECT_EQ(done.get_future().get(), "hello");
    sleep(1);
    std::cout<< "done-c " <<std::endl;
    MqttClient mqttClient{"hello", "127.0.0.1", 1883, ioc};
    std::cout<< "done-d " <<std::endl;
    int messageReceived = 0;
    mqttClient.Subscribe("hello/world", [&messageReceived, &mqttClient](const WildcardValue& wc, const Message& m){
        EXPECT_EQ(m, "HELLO WORLD");
        std::cout<< "message received:" << m <<std::endl;
        messageReceived ++;
        mqttClient.Disconnect();
        std::cout<< "done1 " <<std::endl;
    });

    mqttClient.Publish("hello/world", "HELLO WORLD");

    std::cout<< "done2 " <<std::endl;
    ioc.run();

    std::cout<< "done3 " <<std::endl;
    EXPECT_EQ(messageReceived, 1);

    sleep(10);
    std::cout<< "done4d " <<std::endl;
    system ("ls -l");
    sleep(10);
    system ("killall mosquitto");

    std::cout<< "done5 " <<std::endl;
//    t1.join();
}

TEST(mqtt, simpleMqttClient) {
    boost::asio::io_context ioc;

    MqttClient mqttClient{"hello", "127.0.0.1", 1883, ioc};
    //mqttClient.Publish("hello/world", "HELLO WORLD");
    mqttClient.Subscribe("hello/world", [](const WildcardValue& wc, const Message& m){
        std::cout << "1.WildcardValue received: " << wc << std::endl;
        std::cout << "1.Message received: " << m << std::endl;
    });
    mqttClient.Subscribe("hello/world2", [](const WildcardValue& wc, const Message& m){
        std::cout << "2.WildcardValue received: " << wc << std::endl;
        std::cout << "2.Message received: " << m << std::endl;
    });
    ioc.run();
}

