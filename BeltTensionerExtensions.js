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

    var ctlCode = 1;

    var lsCtl = 3;
    var rsCtl = 4;
    var lwCtl = 5;
    var rwCtl = 6;

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
        var above127Ctl = 0;
        command = command.concat(String.fromCharCode(ctlCode));
        command = command.concat(String.fromCharCode(servoIndex));
        if (val > 127) {
            val -= 127;
            command = command.concat(String.fromCharCode(above127Ctl));
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
        lsCtl,
        rsCtl,
        lwCtl,
        rwCtl,
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
        ctlCode,
        lsnPos,
        rsnPos,
        lwnPos,
        rwnPos,
        concatCommand,
        applyNeutralOffset,
        getMaxTensionPos,
        getMinTensionPos,
        getCommandValFromAbs
    }

}
