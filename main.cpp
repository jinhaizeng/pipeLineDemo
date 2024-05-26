#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>

// Caps类用于描述媒体类型和格式
class Caps {
public:
    std::string mime_type;
    Caps(const std::string& mime) : mime_type(mime) {}
};

// Pad类用于表示Element的输入输出接口
class Pad : public std::enable_shared_from_this<Pad> {
public:
    enum class Type {
        SRC,
        SINK,
        REQUEST
    };

    enum class State {
        STOPED,
        PAUSED,
        PLAYING
    };

    Type type;
    State state;
    Caps caps; // 与此Pad相关联的Caps
    std::function<void(const std::string&)> on_data; //数据处理函数

    // 使用容器存储与之链接的Pad，直冲多对多链接
    std::vector<std::weak_ptr<Pad>> peers;

    Pad(Type t, const Caps& c) : type(t), caps(c), state(State::STOPED) {}

    // 激活Pad
    void activite() {
        state = State::PLAYING;
        std::cout << "Pad activated" << std::endl;
    }

    // 停用Pad
    void deactivite() {
        state = State::STOPED;
        std::cout << "Pad activated" << std::endl;
    }

    // 处理数据的函数
    void push_data(const std::string& data) {
        if (state == State::PLAYING && on_data) {
            on_data(data);
        }
    }

    // 发送时间
    void send_event(const std::string& event) {
        // 处理事件，例如“EOS”（End of Stream）
        std::cout << "Event received: " << event << std::endl;
        if (event == "EOS") {
            deactivite();
        }
    }

    // 支持多对多链接的link
    void link(const std::shared_ptr<Pad> other) {
        if (this->caps.mime_type == other->caps.mime_type) {
            this->peers.push_back(shared_from_this());

            // 设置数据处理函数，当数据到达时，直接推送到链接的Pad
            this->on_data = [this](const std::string& data) {
                for (auto& weak_peer : this->peers) {
                    if (auto peer = weak_peer.lock()) {
                        peer->push_data(data);
                    }
                }
            };
        }
    }
};

// Element类用于表示处理单元
class Element {
public:
    std::vector<std::shared_ptr<Pad>> pads; // Element包含的Pads

    virtual void link(Element& other) {
        for (auto& pad : pads) {
            if (pad->type == Pad::Type::SRC) {
                for (auto& other_pad : other.pads) {
                    if (other_pad->type == Pad::Type::SINK) {
                        pad->link(other_pad);
                    }
                }
            }
        }
    }

    virtual void process(const std::string& data) {
        // 默认的处理函数，子类可以重载这个函数来实现待定的处理器
        for (auto& pad : pads) {
            if (pad->type == Pad::Type::SRC) {
                pad->push_data(data);
            }
        }
    }

    // 激活Element的所有Pad
    void activite_pads() {
        for (auto& pad : pads) {
            pad->activite();
        }
    }

    // 停用Element的所有Pad
    void deactivite() {
        for (auto& pad : pads) {
            pad->deactivite();
        }
    }

    // 处理事件的函数
    virtual void on_event(const std::string& event) {
        // 默认的事件处理函数，子类可以重载这个函数来时间特定的事件处理逻辑
        std::cout << "Event received in Element: " << event << std::endl;
        for (auto& pad : pads) {
            pad->send_event(event);
        }
    }
}
int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
