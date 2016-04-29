#ifndef PRODUCER
#define PRODUCER
#include <memory>

struct Producer
{
    virtual void request(unsigned int n) = 0;
};

using ProducerRef = std::shared_ptr<Producer>;
#endif // PRODUCER

