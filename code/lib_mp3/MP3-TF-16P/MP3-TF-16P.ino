#include "./lib/mp3tf16p.h"

#define led 13

// MP3 Player
MP3Player mp3(0, 1);

void setup(void)
{
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  mp3.initialize();

  // mp3.player.EQ(DFPLAYER_EQ_BASS);
  // mp3.playTrackNumber(1,25);
  mp3.playTrackNumber(1, 25,false);

  while (!mp3.playCompleted())
  {
    if (digitalRead(led) == HIGH)
    {
      digitalWrite(led, LOW);
    }
    else
    {
      digitalWrite(led, HIGH);
    }

    delay(50);
  }

  // int fileCounts = mp3.player.readFileCounts();
  // Serial.println(fileCounts);

  // int folderCounts = mp3.player.readFolderCounts();
  // Serial.println(folderCounts);

  // mp3.player.volume(20);
  // mp3.player.randomAll();
}

void loop(void)
{
  mp3.serialPrintStatus(MP3_ALL_MESSAGE);
  delay(100);
}