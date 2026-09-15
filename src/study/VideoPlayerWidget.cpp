#include "VideoPlayerWidget.h"
#include "utils/PathUtils.h"
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>
#include <QDebug>
#include <QSettings>
#include <QDesktopServices>
#include <QMap>
#include <QSizePolicy>

VideoPlayerWidget::VideoPlayerWidget(QWidget* parent)
    : QWidget(parent), m_isPlaying(false), m_duration(0), m_currentVolume(70) {
    
    m_supportedFormats << "MP4 (*.mp4)" << "AVI (*.avi)" << "WMV (*.wmv)" 
                       << "MKV (*.mkv)" << "MOV (*.mov)" << "FLV (*.flv)"
                       << "WEBM (*.webm)" << "MP3 (*.mp3)" << "WAV (*.wav)";
    
    setupUI();
    updateDecoderStatus();
}

VideoPlayerWidget::~VideoPlayerWidget() {
    if (m_mediaPlayer) {
        m_mediaPlayer->stop();
        delete m_mediaPlayer;
    }
}

void VideoPlayerWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 视频显示区域（顶部，可伸缩）
    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setMinimumSize(640, 480);
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_videoWidget->setStyleSheet("QVideoWidget { background-color: #000; border: none; }");
    mainLayout->addWidget(m_videoWidget);
    
    // 控制栏（固定高度）
    auto* controlPanel = new QWidget(this);
    controlPanel->setFixedHeight(80);
    controlPanel->setStyleSheet("background: #2c3e50;");
    
    auto* controlLayout = new QHBoxLayout(controlPanel);
    controlLayout->setContentsMargins(10, 8, 10, 8);
    controlLayout->setSpacing(12);
    
    // 打开按钮
    m_openBtn = new QPushButton("📽️ 打开视频", controlPanel);
    m_openBtn->setToolTip("支持 WMV, AVI, MP4 等格式");
    m_openBtn->setStyleSheet(
        "QPushButton { padding: 8px 15px; font-weight: bold; color: white; "
        "background: #3498db; border: none; border-radius: 4px; }"
        "QPushButton:hover { background: #2980b9; }"
    );
    connect(m_openBtn, &QPushButton::clicked, this, &VideoPlayerWidget::onOpenVideo);
    
    // 播放控制按钮样式
    QString controlBtnStyle = 
        "QPushButton { min-width: 40px; min-height: 35px; font-size: 16px; "
        "background: #34495e; color: #ecf0f1; border: none; border-radius: 4px; } "
        "QPushButton:hover { background: #4a5f7a; }"
        "QPushButton:disabled { background: #7f8c8d; }";
    
    m_playBtn = new QPushButton("▶️", controlPanel);
    m_playBtn->setEnabled(false);
    m_playBtn->setToolTip("播放/暂停");
    m_playBtn->setStyleSheet(controlBtnStyle);
    connect(m_playBtn, &QPushButton::clicked, this, &VideoPlayerWidget::onPlayPause);
    
    m_stopBtn = new QPushButton("⏹️", controlPanel);
    m_stopBtn->setEnabled(false);
    m_stopBtn->setToolTip("停止");
    m_stopBtn->setStyleSheet(controlBtnStyle);
    connect(m_stopBtn, &QPushButton::clicked, this, &VideoPlayerWidget::onStop);
    
    // 进度条
    m_positionSlider = new QSlider(Qt::Horizontal, controlPanel);
    m_positionSlider->setEnabled(false);
    m_positionSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 6px; background: #34495e; border-radius: 3px; }"
        "QSlider::handle:horizontal { width: 12px; height: 12px; margin: -3px 0; "
        "background: #3498db; border-radius: 6px; }"
    );
    connect(m_positionSlider, &QSlider::sliderMoved, this, &VideoPlayerWidget::onSliderMoved);
    
    // 时间标签
    m_timeLabel = new QLabel("00:00 / 00:00", controlPanel);
    m_timeLabel->setMinimumWidth(90);
    m_timeLabel->setStyleSheet("font-family: monospace; font-size: 11px; color: #ecf0f1;");
    
    // 音量控制
    auto* volumeGroup = new QWidget(controlPanel);
    volumeGroup->setFixedWidth(40);
    auto* volumeLayout = new QVBoxLayout(volumeGroup);
    volumeLayout->setSpacing(2);
    volumeLayout->setContentsMargins(0, 0, 0, 0);
    
    auto* volumeLabel = new QLabel("🔊", volumeGroup);
    volumeLabel->setAlignment(Qt::AlignCenter);
    volumeLabel->setStyleSheet("color: #ecf0f1; font-size: 12px;");
    
    m_volumeSlider = new QSlider(Qt::Vertical, volumeGroup);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setFixedHeight(40);
    m_volumeSlider->setToolTip("音量");
    m_volumeSlider->setStyleSheet(
        "QSlider::groove:vertical { width: 4px; background: #34495e; border-radius: 2px; }"
        "QSlider::handle:vertical { width: 10px; height: 10px; margin: 0 -3px; "
        "background: #3498db; border-radius: 5px; }"
    );
    connect(m_volumeSlider, &QSlider::valueChanged, this, &VideoPlayerWidget::onVolumeChanged);
    
    volumeLayout->addWidget(volumeLabel);
    volumeLayout->addWidget(m_volumeSlider);
    
    // 组装控制栏
    controlLayout->addWidget(m_openBtn);
    controlLayout->addSpacing(20);
    controlLayout->addWidget(m_playBtn);
    controlLayout->addWidget(m_stopBtn);
    controlLayout->addSpacing(15);
    controlLayout->addWidget(m_positionSlider, 1);
    controlLayout->addSpacing(10);
    controlLayout->addWidget(m_timeLabel);
    controlLayout->addSpacing(10);
    controlLayout->addWidget(volumeGroup);
    
    mainLayout->addWidget(controlPanel);
    
    // 状态栏（底部）
    auto* statusPanel = new QWidget(this);
    statusPanel->setFixedHeight(50);
    statusPanel->setStyleSheet("background: #ecf0f1; border-top: 1px solid #bdc3c7;");
    
    auto* statusLayout = new QHBoxLayout(statusPanel);
    statusLayout->setContentsMargins(10, 5, 10, 5);
    statusLayout->setSpacing(10);
    
    m_decoderStatusLabel = new QLabel("检测解码器中...", statusPanel);
    m_decoderStatusLabel->setStyleSheet("font-size: 11px; color: #2c3e50;");
    
    auto* helpBtn = new QPushButton("?", statusPanel);
    helpBtn->setFixedSize(22, 22);
    helpBtn->setToolTip("查看支持的格式和解决方案");
    helpBtn->setStyleSheet(
        "QPushButton { background: #7f8c8d; color: white; border-radius: 11px; "
        "font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background: #34495e; }"
    );
    connect(helpBtn, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "视频播放帮助",
            "<h3>支持的格式</h3>"
            "<b>✅ 无需解码器（直接播放）：</b>"
            "<ul><li>WMV, AVI, MP3, WAV</li></ul>"
            "<b>⚠️ 需要安装解码器：</b>"
            "<ul><li>MP4, MKV, MOV, FLV, WEBM</li></ul>"
            "<h3>安装解码器</h3>"
            "1. 访问 <a href='https://codecguide.com/download_k-lite_codec_pack_basic.htm'>K-Lite 官网</a><br>"
            "2. 下载 <b>Basic</b> 版本<br>"
            "3. 安装时选择 <b>Simple</b> 模式<br>"
            "4. <b>重启</b>本程序即可"
        );
    });
    
    m_fileLabel = new QLabel("未加载视频文件", statusPanel);
    m_fileLabel->setStyleSheet("font-size: 11px; color: #2c3e50;");
    m_fileLabel->setAlignment(Qt::AlignRight);
    
    statusLayout->addWidget(m_decoderStatusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(helpBtn, 0, Qt::AlignRight);
    statusLayout->addSpacing(15);
    statusLayout->addWidget(m_fileLabel);
    
    mainLayout->addWidget(statusPanel);
    
    // 详细信息面板（可折叠）
    m_infoPanel = new QTextEdit(this);
    m_infoPanel->setReadOnly(true);
    m_infoPanel->setFixedHeight(60);
    m_infoPanel->setStyleSheet(
        "QTextEdit { background: #2c3e50; color: #ecf0f1; border: none; "
        "font-family: Consolas; font-size: 10px; padding: 5px; }"
    );
    m_infoPanel->hide();
    mainLayout->addWidget(m_infoPanel);
    
    // 媒体播放器
    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    
    connect(m_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this, &VideoPlayerWidget::onMediaError);
    
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged,
            this, &VideoPlayerWidget::onMediaStatusChanged);
    
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged,
            this, &VideoPlayerWidget::onPlayerStateChanged);
    
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
            this, &VideoPlayerWidget::onPositionChanged);
    
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged,
            this, &VideoPlayerWidget::onDurationChanged);
}

void VideoPlayerWidget::updateDecoderStatus() {
#ifdef Q_OS_WIN
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 
                       QSettings::NativeFormat);
    
    bool hasKLite = false, hasLAV = false;
    
    for (const QString& key : settings.allKeys()) {
        if (key.contains("K-Lite", Qt::CaseInsensitive)) {
            hasKLite = true;
            break;
        }
        if (key.contains("LAV Filters", Qt::CaseInsensitive)) {
            hasLAV = true;
            break;
        }
    }
    
    if (hasKLite || hasLAV) {
        m_decoderStatusLabel->setText("✅ 系统解码器已就绪（支持 MP4/MKV/MOV）");
        m_decoderStatusLabel->setStyleSheet("color: #27ae60; font-weight: bold; padding: 5px;");
    } else {
        m_decoderStatusLabel->setText("⚠️ 未检测到解码器（仅支持 WMV/AVI）");
        m_decoderStatusLabel->setStyleSheet("color: #e74c3c; font-weight: bold; padding: 5px;");
        
        m_infoPanel->show();
        m_infoPanel->setPlainText(
            "[系统检测] 未找到 K-Lite Codec Pack 或 LAV Filters。\n"
            "若要播放 MP4/MKV/MOV 等格式，请点击右侧 ? 按钮查看安装方法。\n"
            "建议：在 learning resources/videos 中优先使用 WMV 或 AVI 格式。"
        );
    }
#else
    m_decoderStatusLabel->setText("ℹ️ 非 Windows 系统");
    m_decoderStatusLabel->setStyleSheet("color: #3498db; padding: 5px;");
#endif
}

QString VideoPlayerWidget::getFormatName(const QString& filePath) const {
    QString ext = QFileInfo(filePath).suffix().toLower();
    static QMap<QString, QString> formatMap = {
        {"wmv", "Windows Media Video"},
        {"avi", "Audio Video Interleave"},
        {"mp4", "MPEG-4"},
        {"mkv", "Matroska"},
        {"mov", "QuickTime"},
        {"flv", "Flash Video"},
        {"webm", "WebM"},
        {"mp3", "MP3 Audio"},
        {"wav", "Wave Audio"}
    };
    return formatMap.value(ext, "未知格式");
}

bool VideoPlayerWidget::openVideo(const QString& filePath) {
    QString path = filePath.trimmed();
    
    // 1. 空路径检查
    if (path.isEmpty()) {
        m_fileLabel->setText("未加载视频文件");
        m_fileLabel->setStyleSheet("color: #7f8c8d; padding: 10px;");
        return false;
    }
    
    // 2. 文件存在性检查（最关键）
    if (!QFile::exists(path)) {
        QFileInfo fi(path);
        m_fileLabel->setText(QString("❌ 文件不存在\n%1\n绝对路径: %2").arg(path).arg(fi.absoluteFilePath()));
        m_fileLabel->setStyleSheet("color: #e74c3c; padding: 10px;");
        m_infoPanel->append(QString("【错误】文件不存在: %1").arg(path));
        return false;
    }
    
    // 3. 获取文件信息
    QFileInfo fileInfo(path);
    QString ext = fileInfo.suffix().toLower();
    QString formatName = getFormatName(path);
    
    // 4. 仅打印调试信息，不强校验后缀（因为过滤器已处理）
    qDebug() << "【VideoPlayer】尝试加载文件:";
    qDebug() << "  路径:" << path;
    qDebug() << "  绝对路径:" << fileInfo.absoluteFilePath();
    qDebug() << "  后缀:" << ext;
    qDebug() << "  格式:" << formatName;
    
    // 5. 清除旧内容并显示加载信息
    m_infoPanel->show();
    m_infoPanel->clear();
    m_infoPanel->append(QString("【加载视频】%1").arg(fileInfo.fileName()));
    m_infoPanel->append(QString("格式: %1 (.%2)").arg(formatName).arg(ext));
    
    if (ext == "mp4" || ext == "mkv" || ext == "mov") {
        m_infoPanel->append("⚠️ 该格式可能需要系统解码器支持");
    }
    
    // 6. 加载媒体文件（核心操作）
    QUrl mediaUrl = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
    m_mediaPlayer->setMedia(mediaUrl);
    
    // 7. 更新UI状态
    m_fileLabel->setText(fileInfo.fileName());
    m_fileLabel->setStyleSheet("color: #27ae60; padding: 10px; font-weight: bold;");
    
    // 8. 设置音量
    if (m_volumeSlider) {
        m_currentVolume = m_volumeSlider->value();
    }
    m_mediaPlayer->setVolume(m_currentVolume);
    
    // 9. 启用控制按钮
    m_playBtn->setEnabled(true);
    m_stopBtn->setEnabled(true);
    m_positionSlider->setEnabled(true);
    
    emit videoLoaded(fileInfo.fileName());
    
    // 10. 同步音量滑块
    if (m_volumeSlider) {
        m_volumeSlider->setValue(m_currentVolume);
    }
    
    return true;
}

void VideoPlayerWidget::onOpenVideo() {
    QString defaultDir = PathUtils::getLearningResourcePath("videos");
    if (!QDir(defaultDir).exists()) {
        defaultDir = QDir::homePath();
    }
    
    // 使用最简过滤器 - Qt会自动处理大小写
    QString path = QFileDialog::getOpenFileName(
        this,
        "打开视频文件",
        defaultDir,
        "所有视频 (*.mp4 *.avi *.wmv *.mkv *.mov *.flv *.webm *.mp3 *.wav);;"
        "所有文件 (*.*)"
    );
    
    if (path.isEmpty()) return;  // 用户取消
    
    // 调用openVideo并传入路径
    openVideo(path);
}

void VideoPlayerWidget::onPlayPause() {
    if (m_mediaPlayer->media().isNull()) return;
    
    if (m_isPlaying) {
        m_mediaPlayer->pause();
        m_playBtn->setText("▶️");
        m_infoPanel->append("【播放控制】已暂停");
    } else {
        m_mediaPlayer->play();
        m_playBtn->setText("⏸️");
        m_infoPanel->append("【播放控制】正在播放");
    }
    m_isPlaying = !m_isPlaying;
}

void VideoPlayerWidget::onStop() {
    m_mediaPlayer->stop();
    m_playBtn->setText("▶️");
    m_isPlaying = false;
    m_positionSlider->setValue(0);
    m_timeLabel->setText("00:00 / 00:00");
    m_infoPanel->append("【播放控制】已停止");
}

void VideoPlayerWidget::onPositionChanged(qint64 position) {
    if (!m_duration) return;
    
    m_positionSlider->blockSignals(true);
    m_positionSlider->setValue(static_cast<int>((position * 1000) / m_duration));
    m_positionSlider->blockSignals(false);
    
    int pos_sec = static_cast<int>(position / 1000);
    int dur_sec = static_cast<int>(m_duration / 1000);
    m_timeLabel->setText(
        QString("%1:%2 / %3:%4")
        .arg(pos_sec / 60, 2, 10, QChar('0'))
        .arg(pos_sec % 60, 2, 10, QChar('0'))
        .arg(dur_sec / 60, 2, 10, QChar('0'))
        .arg(dur_sec % 60, 2, 10, QChar('0'))
    );
}

void VideoPlayerWidget::onDurationChanged(qint64 duration) {
    m_duration = duration;
    m_positionSlider->setRange(0, 1000);
    m_infoPanel->append(QString("时长: %1 秒").arg(duration / 1000));
}

void VideoPlayerWidget::onSliderMoved(int position) {
    if (!m_duration) return;
    qint64 targetPos = (position * m_duration) / 1000;
    m_mediaPlayer->setPosition(targetPos);
    m_infoPanel->append(QString("【跳转】%1 秒").arg(targetPos / 1000));
}

void VideoPlayerWidget::onVolumeChanged(int volume) {
    m_mediaPlayer->setVolume(volume);
    m_infoPanel->append(QString("【音量】%1%").arg(volume));
}

void VideoPlayerWidget::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    QString statusStr;
    switch (status) {
        case QMediaPlayer::LoadingMedia: statusStr = "加载中..."; break;
        case QMediaPlayer::LoadedMedia: statusStr = "加载完成"; break;
        case QMediaPlayer::BufferingMedia: statusStr = "缓冲中..."; break;
        case QMediaPlayer::BufferedMedia: statusStr = "缓冲完成"; break;
        case QMediaPlayer::EndOfMedia: 
            statusStr = "播放结束";
            onStop();
            break;
        default: return;
    }
    
    if (!statusStr.isEmpty()) {
        m_infoPanel->append(QString("【状态】%1").arg(statusStr));
    }
    
    if (status == QMediaPlayer::LoadedMedia) {
        m_playBtn->setEnabled(true);
        m_stopBtn->setEnabled(true);
        m_positionSlider->setEnabled(true);
    }
}

void VideoPlayerWidget::onPlayerStateChanged(QMediaPlayer::State state) {
    Q_UNUSED(state)
}

// 确保实现 onMediaError 槽函数
void VideoPlayerWidget::onMediaError(QMediaPlayer::Error error) {
    QString errorTitle;
    QString errorDetail;
    QString solution;
    
    switch (error) {
        case QMediaPlayer::ResourceError:
            errorTitle = "资源错误";
            errorDetail = "无法打开媒体文件，可能原因：";
            solution = 
                "1. 文件路径包含中文/特殊字符 → 改用纯英文路径\n"
                "2. 文件被占用 → 关闭其他播放器\n"
                "3. 缺少解码器 → 安装 K-Lite Codec Pack";
            break;
            
        case QMediaPlayer::FormatError:
            errorTitle = "格式不支持";
            errorDetail = "系统缺少该视频的解码器：";
            solution = 
                "解决方案：\n"
                "• 安装 K-Lite Codec Pack Basic（推荐）\n"
                "• 或将视频转换为 WMV/AVI 格式\n"
                "下载地址：https://codecguide.com";
            break;
            
        case QMediaPlayer::ServiceMissingError:
            errorTitle = "服务缺失";
            errorDetail = "Qt Multimedia 插件未正确部署：";
            solution = "请确保 mediaservice 文件夹包含 dsengine.dll";
            break;
            
        default:
            errorTitle = "播放错误";
            errorDetail = m_mediaPlayer->errorString();
            solution = "请检查视频文件完整性";
    }
    
    m_fileLabel->setTextFormat(Qt::RichText);
    m_fileLabel->setText(
        QString("<div style='color:#e74c3c;'>"
                "<h4>❌ %1</h4>"
                "<p>%2</p>"
                "<hr>"
                "<p><b>解决建议：</b></p>"
                "<pre style='margin:0; white-space: pre-wrap;'>%3</pre>"
                "</div>")
        .arg(errorTitle)
        .arg(errorDetail)
        .arg(solution)
    );
    m_fileLabel->setStyleSheet("padding: 12px;");
    
    m_infoPanel->append(QString("\n【播放失败】%1: %2").arg(errorTitle).arg(errorDetail));
    m_infoPanel->append(QString("建议: %1").arg(solution.split("\n").first()));
}