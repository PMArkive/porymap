#pragma once
#ifndef ORDERED_SET_H
#define ORDERED_SET_H

#include <QJsonArray>

template <typename T>
class OrderedSet : public std::set<T>
{
    using std::set<T>::set;

public:
    // Not introduced to std::set until C++20
#if __cplusplus < 202002L
    bool contains(const T& value) const {
        return this->find(value) != this->end();
    }
#endif
    QSet<T> toQSet() const {
        return QSet<T>(this->begin(), this->end());
    }
    static QSet<T> fromQSet(const QSet<T>& set) {
        return OrderedSet<T>(set.begin(), set.end());
    }
    bool isEmpty() const {
        return this->empty();
    }
};

#endif // ORDERED_SET_H
