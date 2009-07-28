#! /bin/bash
./phoneme_transcription.py filenames.txt phoneme_transcription_raw.nc
normalise_inputs.sh phoneme_transcription_raw.nc
./phoneme_transcription.py -m filenames.txt phoneme_transcription_mfc.nc
normalise_inputs.sh phoneme_transcription_mfc.nc
