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

var ctlCount = 5;
var valCount = 255 - ctlCount;

var lsCtl = 0;
var rsCtl = 1;
var lwCtl = 2;
var rwCtl = 3;

function calcAbsFromPct(pct){
  return (pct / 100) * (valCount);
}

function getCommandValFromAbs(abs){
  return abs + ctlCount;
}

var lsnPos = calcAbsFromPct(lsn);
var rsnPos = calcAbsFromPct(rsn);
var lwnPos = calcAbsFromPct(lwn);
var rwnPos = calcAbsFromPct(rwn);

function concatCommand(command, servoIndex, val){
  var above127Ctl = 4;
  command = command.concat(String.fromCharCode(servoIndex));
  if (val > 127) {
    val -= 127;
    command = command.concat(String.fromCharCode(above127Ctl));
  }
  val = Math.min(~~val, 127);
  command = command.concat(String.fromCharCode(~~val));
  return command;
}

function applyNeutralOffset(offsetPos, decimalValue){
  decimalValue = Math.max(Math.min(decimalValue, 1), -1);
  var posValue = offsetPos;
  if (decimalValue > 0) {
    posValue = offsetPos + ((valCount - offsetPos) * decimalValue)
  } else if (decimalValue < 0) {
    posValue = offsetPos - (offsetPos * Math.abs(decimalValue))
  }
  return posValue;
}

function getMaxTensionPos(offsetPos, maxVal){
  return applyNeutralOffset(offsetPos, (maxVal / 100));
}

function getMinTensionPos(offsetPos, minVal){
  return applyNeutralOffset(offsetPos, (minVal / 100)*-1);
}

if ($prop('Settings.TestNeutralTensions')) {
  var command = "";
  command = concatCommand(command, lsCtl, getCommandValFromAbs(lsnPos));
  command = concatCommand(command, rsCtl, getCommandValFromAbs(rsnPos));
  command = concatCommand(command, lwCtl, getCommandValFromAbs(lwnPos));
  command = concatCommand(command, rwCtl, getCommandValFromAbs(rwnPos));
  return command;
} else if ($prop('Settings.TestMaxTensions')) {
  var command = "";
  command = concatCommand(command, lsCtl, getCommandValFromAbs(getMaxTensionPos(lsnPos, lsMax)));
  command = concatCommand(command, rsCtl, getCommandValFromAbs(getMaxTensionPos(rsnPos, rsMax)));
  command = concatCommand(command, lwCtl, getCommandValFromAbs(getMaxTensionPos(lwnPos, lwMax)));
  command = concatCommand(command, rwCtl, getCommandValFromAbs(getMaxTensionPos(rwnPos, rwMax)));
  return command;
}else if ($prop('Settings.TestMinTensions')) {
  var command = "";
  command = concatCommand(command, lsCtl, getCommandValFromAbs(getMinTensionPos(lsnPos, lsMin)));
  command = concatCommand(command, rsCtl, getCommandValFromAbs(getMinTensionPos(rsnPos, rsMin)));
  command = concatCommand(command, lwCtl, getCommandValFromAbs(getMinTensionPos(lwnPos, lwMin)));
  command = concatCommand(command, rwCtl, getCommandValFromAbs(getMinTensionPos(rwnPos, rwMin)));
  return command;
}
