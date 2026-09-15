#pragma once

/**
 * @brief 节点类模板 - 支持自定义数据
 * @tparam DataType 节点数据类型
 */
template<typename DataType = std::string>
class Node {
public:
    explicit Node(int id, const DataType& data = DataType{})
        : m_id(id), m_data(data) {}

    int id() const { return m_id; }
    DataType& data() { return m_data; }
    const DataType& data() const { return m_data; }
    void setData(const DataType& data) { m_data = data; }

    void setPosition(double x, double y) { m_x = x; m_y = y; }
    double x() const { return m_x; }
    double y() const { return m_y; }

private:
    int m_id;
    DataType m_data;
    double m_x{0}, m_y{0};
};
