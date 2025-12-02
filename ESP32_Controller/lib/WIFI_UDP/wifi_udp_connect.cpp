#include "wifi_udp_connect.hpp"

namespace wifi_udp_connect
{
WiFiUDP Udp;

bool UdpUpload(IPAddress ip, uint16_t port, uint8_t * data, int len)
{
    // 检测 WiFi 连接状态
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    } else {
        Udp.beginPacket(ip, port);
        Udp.write(data, len);
        if (Udp.endPacket()) {
            return true;
        } else {
            return false;
        }
    }
}
}  // namespace wifi_udp_connect
