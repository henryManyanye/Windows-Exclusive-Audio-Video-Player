when playing audio using bluetooth, it doesnt play audio after 2 or more pauses
 but if u stop the currently silent song and then re-initialize the player, it works
  maybe try re-initializing the player after pausing BUT seeking to the position of the last read byte before rendering audio.
   ALSO USE THE SAME STRATEGY WHEN WHEN THE AUDIO RENDERING DEVICE CHANGES OR ITS PROPERTIES CHANGE

when playing audio using bluetooth, it still glitches. Even when not playing on the commandline
