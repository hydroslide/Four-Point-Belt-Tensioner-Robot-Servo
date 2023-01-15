// Place this code at D:\Program Files (x86)\SimHub\JavascriptExtensions
GetTensionUtils = function () {

    var lsn = [$prop('Settings.LeftShoulderNeutral')];
    var rsn = [$prop('Settings.RightShoulderNeutral')];
    var lwn = [$prop('Settings.LeftWaistNeutral')];
    var rwn = [$prop('Settings.RightWaistNeutral')];

    var lsMax = [$prop('Settings.LeftShoulderMax')];
    var rsMax = [$prop('Settings.RightShoulderMax')];
    var lwMax = [$prop('Settings.LeftWaistMax')];
    var rwMax = [$prop('Settings.RightWaistMax')];

    var lsMin = [$prop('Settings.LeftShoulderMin')];
    var rsMin = [$prop('Settings.RightShoulderMin')];
    var lwMin = [$prop('Settings.LeftWaistMin')];
    var rwMin = [$prop('Settings.RightWaistMin')];

    var ctlCount = 2;
    var valCount = 255 - ctlCount;

                             root["neutralSet"] = 0;

    var ctlCodes = {
        // COntrol Codes
        above127: 0,
        control: 1,
        leftShoulder: 3,
        rightShoulder: 4,
        leftWaist: 5,
        rightWaist: 6,
        forceSleep: 7,
        setRedLeft: 8,
        setGreenLeft: 9,
        setBlueLeft: 10,
        setRedRight: 11,
        setGreenRight: 12,
        setBlueRight: 13,
        leftShoulderNeutral: 14,
        rightShoulderNeutral: 15,                                                                                     
        leftWaistNeutral: 16,
        rightWaistNeutral: 17,
        resetCurrentDegrees:18,
        findTheLimitsCtl:19
    }

    /*
    function defaultToNeutralIfNotRunning(command) {
        if ($prop('DataCorePlugin.GameRunning')!=0) {
          root["neutralSet"] = 0;
        }
        else {
          //return "Neutral Set: "+root["neutralSet"];  
          command = "";
          if (root["neutralSet"]<50) {
            //root["neutralSet"] += 1;
            //return "CCC";
            // command = concatCommand(command, ctlCodes.leftShoulderNeutral, getCommandValFromAbs(lsnPos));
            // command = concatCommand(command, ctlCodes.rightShoulderNeutral, getCommandValFromAbs(rsnPos));
            // command = concatCommand(command, ctlCodes.leftWaistNeutral, getCommandValFromAbs(lwnPos));
            // command = concatCommand(command, ctlCodes.rightWaistNeutral, getCommandValFromAbs(rwnPos));
            command = concatCommand(command, ctlCodes.leftShoulder, getCommandValFromAbs(lsnPos));
            command = concatCommand(command, ctlCodes.rightShoulder, getCommandValFromAbs(rsnPos));
            command = concatCommand(command, ctlCodes.leftWaist, getCommandValFromAbs(lwnPos));
            command = concatCommand(command, ctlCodes.rightWaist, getCommandValFromAbs(rwnPos));
          }
        }
        return command;
      }
*/
    function calcAbsFromPct(pct) {
        return (pct / 100) * (valCount);
    }

    function getCommandValFromAbs(abs) {
        return abs + ctlCount;
    }

    var lsnPos = calcAbsFromPct(lsn);
    var rsnPos = calcAbsFromPct(rsn);
    var lwnPos = calcAbsFromPct(lwn);
    var rwnPos = calcAbsFromPct(rwn);

    function concatCommand(command, servoIndex, val) {
        command = command.concat(String.fromCharCode(ctlCodes.control));
        command = command.concat(String.fromCharCode(servoIndex));
        if (val > 127) {
            val -= 127;
            command = command.concat(String.fromCharCode(ctlCodes.above127));
        }
        val = Math.min(~~val, 127);
        command = command.concat(String.fromCharCode(~~val));
        return command;
    }

    function applyNeutralOffset(offsetPos, decimalValue) {
        decimalValue = Math.max(Math.min(decimalValue, 1), -1);
        var posValue = offsetPos;
        if (decimalValue > 0) {
            posValue = offsetPos + ((valCount - offsetPos) * decimalValue)
        } else if (decimalValue < 0) {
            posValue = offsetPos - (offsetPos * Math.abs(decimalValue))
        }
        return posValue;
    }

    function getMaxTensionPos(offsetPos, maxVal) {
        return applyNeutralOffset(offsetPos, (maxVal / 100));
    }

    function getMinTensionPos(offsetPos, minVal) {
        return applyNeutralOffset(offsetPos, (minVal / 100) * -1);
    }

    return {
        ctlCodes,
        lsn,
        rsn,
        lwn,
        rwn,
        lsMax,
        rsMax,
        lwMax,
        rwMax,
        lsMin,
        rsMin,
        lwMin,
        rwMin,
        ctlCount,
        valCount,
        lsnPos,
        rsnPos,
        lwnPos,
        rwnPos,
        concatCommand,
        applyNeutralOffset,
        getMaxTensionPos,
        getMinTensionPos,
        getCommandValFromAbs,
    }

}
