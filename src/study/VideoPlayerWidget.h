#pragma once

#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QTextEdit>
#include <QStringList>

/**
 * @brief 视频播放器 - 支持MP4/AVI/WMV等格式
 */
class VideoPlayerWidget : public QWidget {
    Q_OBJECT

public:
    explicit VideoPlayerWidget(QWidget* parent = nullptr);
    ~VideoPlayerWidget();

    bool openVideo(const QString& filePath = QString());

signals:
    void videoLoaded(const QString& fileName);
    void videoLoadError(const QString& error);

private slots:
    void onOpenVideo();
    void onPlayPause();
    void onStop();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onSliderMoved(int position);
    void onVolumeChanged(int volume);
    void onMediaError(QMediaPlayer::Error error);  // 确保声明正确
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlayerStateChanged(QMediaPlayer::State state);
    void updateDecoderStatus();

private:
    void setupUI();
    QString getFormatName(const QString& filePath) const;

    QMediaPlayer* m_mediaPlayer = nullptr;
    QVideoWidget* m_videoWidget = nullptr;
    
    QPushButton* m_openBtn = nullptr;
    QPushButton* m_playBtn = nullptr;
    QPushButton* m_stopBtn = nullptr;
    QSlider* m_positionSlider = nullptr;
    QSlider* m_volumeSlider = nullptr;
    QLabel* m_timeLabel = nullptr;
    QLabel* m_fileLabel = nullptr;
    QLabel* m_decoderStatusLabel = nullptr;
    QTextEdit* m_infoPanel = nullptr;
    
    bool m_isPlaying = false;
    qint64 m_duration = 0;
    QStringList m_supportedFormats;
    int m_currentVolume = 70; // 当前音量（默认70%）
};