@Echo off
cls
Echo *********************************************
Echo Step 1 - Burning the Audio Track to your CDR
Echo *********************************************
Echo Insert a Blank CD into CD Recorder
pause
cdrecord -dev=0,1,0 -multi -audio -speed=4 audio.raw
Echo ***********************************
Echo Step 2. - Burning Data Track
Echo ***********************************
Echo Make sure the CD you burned the Audio Track
Echo too in step one is in your burner...
pause
cdrecord -dev=0,1,0 -xa1 -speed=4 data.iso
cdrecord -dev=0,1,0 -eject
Echo *********************
Echo CD BURNING COMPLETED!
Echo *********************
:end
