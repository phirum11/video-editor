#include <QCoreApplication>
#include <QDebug>
#include "core/timeline/models/TimelineClipModel.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    TimelineClipModel model;
    
    TimelineClip clip1;
    clip1.clipName = "EP003";
    clip1.startSeconds = 0;
    clip1.durationSeconds = 185;
    clip1.trackIndex = 0;
    model.updateClip(0, clip1);
    
    TimelineClip clip2;
    clip2.clipName = "EP002";
    clip2.startSeconds = 180;
    clip2.durationSeconds = 220;
    clip2.trackIndex = 2;
    model.updateClip(1, clip2);
    
    TimelineClip clip3;
    clip3.clipName = "EP001";
    clip3.startSeconds = 480;
    clip3.durationSeconds = 266;
    clip3.trackIndex = 2;
    model.updateClip(2, clip3);

    qDebug() << "Before compact:";
    for (int i=0; i<3; ++i) qDebug() << model.clipAt(i).clipName << "Track:" << model.clipAt(i).trackIndex;
    
    model.compactTracks();
    
    qDebug() << "After compact:";
    for (int i=0; i<3; ++i) qDebug() << model.clipAt(i).clipName << "Track:" << model.clipAt(i).trackIndex;
    
    return 0;
}
