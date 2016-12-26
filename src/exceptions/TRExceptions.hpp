#ifndef TREXCEPTIONS_H
#define TREXCEPTIONS_H
#include <exception>

struct TRException: public std::exception
{
    virtual const char* what() const noexcept
    {
        return "TinyRxCpp exception.";
    }
};

struct SlowSubscriberException : public TRException
{
    virtual const char* what() const noexcept
    {
        return "Subscriber is too slow.";
    }
};

struct BadStreamException : public TRException
{
    virtual const char* what() const noexcept
    {
        return "Stream is broken.";
    }
};

struct StreamNotOpenException : public TRException
{
    virtual const char* what() const noexcept
    {
        return "Stream is not open.";
    }
};

struct NullPointerException : public TRException
{
    virtual const char* what() const noexcept
    {
        return "Object is null.";
    }
};
#endif // TREXCEPTIONS_H
