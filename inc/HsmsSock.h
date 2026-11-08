#ifndef INCLUDED_HSMSSOCK_H
#define INCLUDED_HSMSSOCK_H
class HsmsSock {
public:
    HsmsSock() = default;
    virtual ~HsmsSock() = default;
    void OnConnect();
};
#endif
