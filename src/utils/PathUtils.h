#pragma once

#include <QString>
#include <QDir>
#include <QCoreApplication>

class PathUtils {
public:
    // 获取学习资源目录（自动创建）
    static QString getLearningResourcePath(const QString& subDir) {
        QStringList searchPaths;
        searchPaths << QCoreApplication::applicationDirPath() + "/../learning resources/" + subDir  // 开发环境
                  << QCoreApplication::applicationDirPath() + "/learning resources/" + subDir       // 发布环境
                  << QDir::currentPath() + "/learning resources/" + subDir;                        // 备用
        
        for (const QString& path : searchPaths) {
            if (QDir(path).exists()) {
                return path;
            }
        }
        
        // 如果都不存在，创建在第一个位置
        QString createPath = searchPaths.first();
        QDir().mkpath(createPath);
        return createPath;
    }

    // 不区分大小写的文件后缀检查
    static bool hasExtension(const QString& filePath, const QStringList& extensions) {
        QString ext = QFileInfo(filePath).suffix().toLower();
        for (const QString& validExt : extensions) {
            if (ext == validExt.toLower()) {
                return true;
            }
        }
        return false;
    }
};