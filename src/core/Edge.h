#pragma once

/**
 * @brief 边类模板 - 支持权重和自定义数据
 * @tparam WeightType 权重类型
 */
template<typename WeightType = int>
class Edge {
public:
    Edge(int from, int to, WeightType weight = WeightType{})
        : m_from(from), m_to(to), m_weight(weight) {}

    int from() const { return m_from; }
    int to() const { return m_to; }
    WeightType weight() const { return m_weight; }
    void setWeight(WeightType w) { m_weight = w; }

    bool operator<(const Edge& other) const {
        return m_weight < other.m_weight;
    }

private:
    int m_from, m_to;
    WeightType m_weight;
};
