#pragma once
#include <string>

template<typename T, int off>
T& dAccess(void* ptr) {
    return *(T*)(((uintptr_t)ptr) + off);
}

enum class PacketReliability { Relible, RelibleOrdered };
class BinaryStream {
public:
    std::string& getRaw() {
        return *dAccess<std::string*, 88>(this);
    }
};
class Packet {
public:
    unsigned unk2 = 2;                                 // 8
    PacketReliability reliableOrdered = PacketReliability::RelibleOrdered; // 12
    unsigned char clientSubId = 0;                                 // 16
    unsigned unk24 = 0;                                 // 24
    unsigned compressible = 0;                                 // 32
    inline Packet(unsigned compress) : compressible(compress) {}
    inline Packet() {}
    inline virtual ~Packet() {}
    virtual int getId() const = 0;
    virtual std::string getName() const = 0;
    virtual void write(BinaryStream&) const = 0;
    virtual void dummyread() = 0;
    virtual bool disallowBatching() const = 0;
};
template<int pid, bool batching = true, bool compress = true>
class MyPkt :public Packet {
public:
    std::string_view view;
    MyPkt() {
        compressible = compress;
    }
    MyPkt(std::string_view sv) {
        view = sv;
        compressible = compress;
    }
    inline virtual ~MyPkt() {}
    virtual int getId() const {
        return pid;
    }
    virtual std::string getName() const {
        return "MyPkt";
    }
    virtual void write(BinaryStream& bs) const {
        bs.getRaw().append(view);
    }
    virtual void dummyread() {}
    virtual bool disallowBatching() const { return !batching; }
};