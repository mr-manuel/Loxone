// written by mr-manuel
// Github repository: https://github.com/mr-manuel/Loxone
// last change: 2023.08.22

// ---------- donation ----------
// If you find this script useful and you want to donate something
// you find a PayPal link in my GitHub profile: https://github.com/mr-manuel

// ---------- applicable for ----------
// fan control with 0 = air exhaust, 2.5 = off, 5 = air supply

// ---------- how to use ----------
// Inputs
// I1: connect F (Fan) from Room Ventilation Controller - fan speed 0 - 100 %
// I2: connect Fea (Fan exhaust air) from Room Ventilation Controller - fan speed exhaust 0 - 10ö %
// I3: connect Fsa (Fan supply air) from Room Ventilation Controller - fan speed supply 0 - 100 %
// I4: connect He (Heat exchanger) from Room Ventilation Controller - heat exchange 0 = off; 1 = on
// I5: 0 = normal operation
//     1 = O1 exhaust only, O2 supply only
//     2 = O1 supply only, O2 exhaust only
// Outputs
// O1: connect first (third, fith, ...) ventilation from a pair to an analog 0-10 V output (only 0-5 V are used; make sure to limit to max. 5 V)
// O2: connect second (fourth, sixth, ...) ventilation from a pair to an analog 0-10 V output (only 0-5 V are used; make sure to limit to max. 5 V)

float airExhaust,airSupply;
int ai1,ai2,ai3,ai4,currentMode,runTime,runTimeHeatExchange,runTimeNoHeatExchange,sleepTime,i;


// ---------- edit options here | start ----------
// HERE you can edit the default settings, if you want to
// set runtime for heat exchange (default: 63)
runTimeHeatExchange = 63;
// set runtime for no heat exchange (default: 1188)
runTimeNoHeatExchange = 1188;
// set fan running out time when changing direction/mode (default: 12)
sleepTime = 12;
// ---------- edit options here | end ----------


int functionMode(int currentMode) {
    int ai1,ai2,ai3,ai4,ai5,nEvents;

    // check if changes where made to prevent if checks to be executed on no event
    nEvents = getinputevent();
    if(nEvents){
        //printf("nEvents changed: %x", nEvents);
        ai1 = getinput(0);
        ai2 = getinput(1);
        ai3 = getinput(2);
        ai4 = getinput(3);
        ai5 = getinput(4);


        if(ai5 == 1 && ai4 == 1 && ai1 > 0 && ai2 > 0 && ai3 > 0){
            // normal operation: heat exchange
            //printf("Ventilation Program: change detected - switch to heat exchange (0)");
            setoutput(12, 0);
            return 0;
        } else if(ai5 == 1 && ai4 == 0 && ai1 > 0 && ai2 > 0 && ai3 > 0) {
            // normal operation: no heat exchange
            //printf("Ventilation Program: change detected - switch to no heat exchange (1)");
            setoutput(12, 1);
            return 1;
        //} else if(ai5 == 1 && ai4 == 0 && ai1 > 0 && ai2 > 0 && ai3 == 0) {
        } else if(ai5 == 1 && ai1 > 0 && ai2 > 0 && ai3 == 0) {
            // normal operation: exhaust
            //printf("Ventilation Program: change detected - switch to exhaust (2)");
            setoutput(12, 2);
            return 2;
        //} else if(ai5 == 1 && ai4 == 0 && ai1 > 0 && ai2 == 0 && ai3 > 0) {
        } else if(ai5 == 1 && ai1 > 0 && ai2 == 0 && ai3 > 0) {
            // normal operation: supply
            //printf("Ventilation Program: change detected - switch to supply (3)");
            setoutput(12, 3);
            return 3;
        } else if(ai5 == 2){
            // central operation: exhaust
            //printf("Ventilation Program: change detected - switch to CENTRAL exhaust (2)");
            setoutput(12, 102);
            return 2;
        } else if(ai5 == 3){
            // central operation: supply
            //printf("Ventilation Program: change detected - switch to CENTRAL supply (3)");
            setoutput(12, 103);
            return 3;
        } else {
            // default: heat exchange (for example if fan speed is 0)
            //printf("Ventilation Program: change detected - switch to DEFAULT heat exchange (0)");
            setoutput(12, 100);
            return 0;
        }
    } else {
        // no change
        ////printf("Ventilation Program: no change detected");
        return currentMode;
    }
}

// set currentMode for first run
currentMode = functionMode(0);

while(TRUE){

    ai1 = getinput(0);

    // fan speed
    if(ai1 < 1){
        // turn fan off, if 0 %
        airExhaust = 2.5;
        airSupply = 2.5;
    } else if(ai1 > 99){
        // set fan to max, if 100 %
        airExhaust = 0;
        airSupply = 5;
    } else {
        // calculate fan speed
        airExhaust = 0 - ( ai1 * 1.2 / 100 ) + 2.2;
        airSupply = ( ai1 * 1.2 / 100 ) + 2.8;
    }

    currentMode = functionMode(currentMode);
    if(currentMode == 0 || currentMode == 1){
        if(currentMode == 0){
            // normal operation: heat exchange
            // for this time in seconds the fan is blowing in one direction
            runTime = runTimeHeatExchange;
            //printf("Ventilation Program: heat exchange");
        } else {
            // normal operation: no exchange
            // for this time in seconds the fan is blowing in one direction
            runTime = runTimeNoHeatExchange;
            //printf("Ventilation Program: no heat exchange");
        }

        while(TRUE){

            // START DIRECTION 1 -----
            setoutput(0,airExhaust);
            setoutput(1,airSupply);

            // for this time in seconds the fan is blowing in one direction
            for(i = 0; i <= runTime; i++){

                // check if operating mode is changed
                currentMode = functionMode(currentMode);
                if( (currentMode == 0 && runTime == runTimeHeatExchange) || (currentMode == 1 && runTime == runTimeNoHeatExchange)){

                    // check if fan speed is changed
                    if(ai1 != getinput(0)){

                        //printf("Ventilation Program: fan speed changed to %d", ai1);
                        ai1 = getinput(0);

                        if(ai1 < 1){
                            // turn fan off, if 0 %
                            airExhaust = 2.5;
                            airSupply = 2.5;
                        } else if(ai1 > 99){
                            // set fan to max, if 100 %
                            airExhaust = 0;
                            airSupply = 5;
                        } else {
                            // calculate fan speed
                            airExhaust = 0 - ( ai1 * 1.2 / 100 ) + 2.2;
                            airSupply = ( ai1 * 1.2 / 100 ) + 2.8;
                        }

                        setoutput(0,airExhaust);
                        setoutput(1,airSupply);
                    }

                    sleep(1000);

                } else {
                    //printf("Ventilation Program: abort - mode changed X1");
                    break;
                }

            }

            // START PAUSE 1 -----
            setoutput(0,2.5);
            setoutput(1,2.5);

            // for this time in seconds the fan is running out
            for(i = 0; i <= sleepTime; i++){
                // check if operating mode is changed
                currentMode = functionMode(currentMode);
                if( (currentMode == 0 && runTime == runTimeHeatExchange) || (currentMode == 1 && runTime == runTimeNoHeatExchange)){
                    sleep(1000);
                } else {
                    //printf("Ventilation Program: abort - mode changed X2");
                    break;
                }
            }

            // START DIRECTION 2 -----
            setoutput(0,airSupply);
            setoutput(1,airExhaust);

            // for this time in seconds the fan is blowing in one direction
            for(i = 0; i <= runTime; i++){

                // check if operating mode is changed
                currentMode = functionMode(currentMode);
                if( (currentMode == 0 && runTime == runTimeHeatExchange) || (currentMode == 1 && runTime == runTimeNoHeatExchange)){

                    // check if fan speed is changed
                    if(ai1 != getinput(0)){

                        //printf("Ventilation Program: fan speed changed to %d", ai1);
                        ai1 = getinput(0);

                        if(ai1 < 1){
                            // turn fan off, if 0 %
                            airExhaust = 2.5;
                            airSupply = 2.5;
                        } else if(ai1 > 99){
                            // set fan to max, if 100 %
                            airExhaust = 0;
                            airSupply = 5;
                        } else {
                            // calculate fan speed
                            airExhaust = 0 - ( ai1 * 1.2 / 100 ) + 2.2;
                            airSupply = ( ai1 * 1.2 / 100 ) + 2.8;
                        }

                        setoutput(0,airSupply);
                        setoutput(1,airExhaust);
                    }

                    sleep(1000);

                } else {
                    //printf("Ventilation Program: abort - mode changed X3");
                    break;
                }
            }

            // START PAUSE 2 -----
            setoutput(0,2.5);
            setoutput(1,2.5);

            // for this time in seconds the fan is running out
            for(i = 0; i <= sleepTime; i++){
                // check if operating mode is changed
                currentMode = functionMode(currentMode);
                if( (currentMode == 0 && runTime == runTimeHeatExchange) || (currentMode == 1 && runTime == runTimeNoHeatExchange)){
                    sleep(1000);
                }
                else {
                    //printf("Ventilation Program: abort - mode changed X4");
                    break;
                }
            }

            // break while loop
            // check if operating mode is changed
            currentMode = functionMode(currentMode);
            if(currentMode != 0 || currentMode != 1){
                //printf("Ventilation Program: abort - mode changed X5");
                break;
            }

        } // while

        //printf("Ventilation Program: mode changed - let run out fan for %d s", sleepTime);
        setoutput(0,2.5);
        setoutput(1,2.5);
        sleep(sleepTime * 1000);

    } else if(currentMode == 2){
        // if currentMode = 2 then exhaust air
        //printf("Ventilation Program: exhaust air");

        // START EXHAUST -----
        setoutput(0,airExhaust);
        setoutput(1,airSupply);

        while(TRUE){

            // check if operating mode is changed
            currentMode = functionMode(currentMode);
            if(currentMode == 2){

                // check if fan speed is changed
                if(ai1 != getinput(0)){

                    //printf("Ventilation Program: fan speed changed to %d", ai1);
                    ai1 = getinput(0);

                    if(ai1 < 1){
                        // turn fan off, if 0 %
                        airExhaust = 2.5;
                        airSupply = 2.5;
                    } else if(ai1 > 99){
                        // set fan to max, if 100 %
                        airExhaust = 0;
                        airSupply = 5;
                    } else {
                        // calculate fan speed
                        airExhaust = 0 - ( ai1 * 1.2 / 100 ) + 2.2;
                        airSupply = ( ai1 * 1.2 / 100 ) + 2.8;
                    }

                    setoutput(0,airExhaust);
                    setoutput(1,airSupply);
                }

                sleep(1000);

            } else {
                //printf("Ventilation Program: abort - mode changed X10");
                break;
            }
        }

        //printf("Ventilation Program: mode changed - let run out fan for %d s", sleepTime);
        setoutput(0,2.5);
        setoutput(1,2.5);
        sleep(sleepTime * 1000);

    } else {
        // if currentMode = 3 then supply air
        //printf("Ventilation Program: supply air");

        // START SUPPLY -----
        setoutput(0,airSupply);
        setoutput(1,airExhaust);

        while(TRUE){

            // check if operating mode is changed
            currentMode = functionMode(currentMode);
            if(currentMode == 3){

                // check if fan speed is changed
                if(ai1 != getinput(0)){

                    //printf("Ventilation Program: fan speed changed to %d", ai1);
                    ai1 = getinput(0);

                    if(ai1 < 1){
                        // turn fan off, if 0 %
                        airExhaust = 2.5;
                        airSupply = 2.5;
                    } else if(ai1 > 99){
                        // set fan to max, if 100 %
                        airExhaust = 0;
                        airSupply = 5;
                    } else {
                        // calculate fan speed
                        airExhaust = 0 - ( ai1 * 1.2 / 100 ) + 2.2;
                        airSupply = ( ai1 * 1.2 / 100 ) + 2.8;
                    }

                    setoutput(0,airSupply);
                    setoutput(1,airExhaust);
                }

                sleep(1000);

            } else {
                //printf("Ventilation Program: abort - mode changed X20");
                break;
            }
        }

        //printf("Ventilation Program: mode changed - let run out fan for %d s", sleepTime);
        setoutput(0,2.5);
        setoutput(1,2.5);
        sleep(sleepTime * 1000);

    }

} // while(TRUE)
