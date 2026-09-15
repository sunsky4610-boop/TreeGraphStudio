#pragma once

#include <string>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include "../core/Graph.h"

/**
 * @brief JSON序列化工具类 - 支持有向图和无向图
 */
class JsonSerializer {
public:
    // 模板化的序列化方法，支持有向图和无向图
    template<typename NodeData = std::string, typename EdgeWeight = int, bool Directed = false>
    static QJsonObject graphToJson(const Graph<NodeData, EdgeWeight, Directed>* graph) {
        QJsonObject json;

        QJsonArray nodesArray;
        // 直接遍历现存节点 ID：删除节点会留下空槽，旧写法 nodeCount()+100 在大量删除后会漏存节点
        for (int i : graph->getAllNodeIds()) {
            if (auto* node = graph->getNode(i)) {
                QJsonObject nodeObj;
                nodeObj["id"] = i;
                nodeObj["label"] = QString::fromStdString(node->data());
                nodeObj["x"] = node->x();
                nodeObj["y"] = node->y();
                nodesArray.append(nodeObj);
            }
        }
        json["nodes"] = nodesArray;

        QJsonArray edgesArray;
        for (const auto& edge : graph->getEdges()) {
            QJsonObject edgeObj;
            edgeObj["from"] = edge->from();
            edgeObj["to"] = edge->to();
            edgeObj["weight"] = edge->weight();
            edgesArray.append(edgeObj);
        }
        json["edges"] = edgesArray;

        json["type"] = Directed ? "directed" : "undirected";

        return json;
    }

    // 模板化的反序列化方法
    template<typename NodeData = std::string, typename EdgeWeight = int, bool Directed = false>
    static void jsonToGraph(const QJsonObject& json, Graph<NodeData, EdgeWeight, Directed>* graph) {
        QJsonArray nodesArray = json["nodes"].toArray();
        std::unordered_map<int, int> idMapping;

        for (const QJsonValue& nodeVal : nodesArray) {
            QJsonObject nodeObj = nodeVal.toObject();
            int oldId = nodeObj["id"].toInt();
            QString label = nodeObj["label"].toString();
            double x = nodeObj["x"].toDouble();
            double y = nodeObj["y"].toDouble();

            int newId = graph->addNode(label.toStdString());
            if (auto* node = graph->getNode(newId)) {
                node->setPosition(x, y);
            }
            idMapping[oldId] = newId;
        }

        QJsonArray edgesArray = json["edges"].toArray();
        for (const QJsonValue& edgeVal : edgesArray) {
            QJsonObject edgeObj = edgeVal.toObject();
            int oldFrom = edgeObj["from"].toInt();
            int oldTo = edgeObj["to"].toInt();
            int weight = edgeObj["weight"].toInt();

            int newFrom = idMapping[oldFrom];
            int newTo = idMapping[oldTo];

            graph->addEdge(newFrom, newTo, weight);
        }
    }

    static bool saveToFile(const QJsonObject& json, const QString& filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }

        QJsonDocument doc(json);
        file.write(doc.toJson());
        file.close();
        return true;
    }

    static QJsonObject loadFromFile(const QString& filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            return QJsonObject();
        }

        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        file.close();

        return doc.object();
    }
};