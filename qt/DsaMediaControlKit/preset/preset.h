#ifndef PRESET_H
#define PRESET_H

#include <QObject>
#include <QMediaPlayer>
#include <QMediaContent>
#include "preset/playlist_settings_old.h"
#include "preset/playlist_old.h"

namespace Preset {

/*
 * A preset is a collection of different sound playlists. It allows to play
 * all playlists with their own settings at once.
*/

class Preset : public QObject
{
    Q_OBJECT
public:
    explicit Preset(QObject *parent = 0);
    explicit Preset(QString name, QObject *parent = 0);
    explicit Preset(QString name, DB::SoundFileRecord *sound_file, QObject *parent = 0);
    explicit Preset(QString name, const QList<DB::SoundFileRecord *> &sound_list, QObject *parent = 0);
    ~Preset();


    QString getName() const;
    void setName(const QString &name);

    QMap<int, PlaylistOld *> getPlaylists() const;
    void setPlaylists(const QMap<int, PlaylistOld *> &playlists);

    /*
     * creat a playlist and add it to the preset.
    */
    PlaylistOld* createPlaylist(const QString &name);
    PlaylistOld* createPlaylist(const QList<DB::SoundFileRecord*> &sound_files);

signals:
    void presetStart();

public slots:
    void startPreset(bool);

private:
    void initComponents();
    void addPlaylist(int id, PlaylistOld* playlist);

    int id_;
    int playlist_id_iterator_;
    QString name_;
    QMap<int, PlaylistOld*> playlists_;

};

} // namespace PRESET

#endif // PRESET_H
