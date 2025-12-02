#ifndef __WIFI_UDP_CONNECT_HPP
#define __WIFI_UDP_CONNECT_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

namespace wifi_udp_connect
{
bool UdpUpload(IPAddress ip, uint16_t port, uint8_t * data, int len);
}

#endif  // __WIFI_UDP_CONNECT_HPP