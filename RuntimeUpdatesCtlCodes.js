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

function SmoothItOut(servoName, val) {
  // Low-pass IIR filtering of left and right tension values
  var key = servoName + 'B4';
  if (null == root[key]) {
    root[key] = val;	// initialize
  }
  var valB4 = root[key]; // previously filtered values
  var tc = 1 + $prop('Settings.smooth');
  valB4 += (val - valB4) / tc;
  root[key] = valB4;
  return valB4;
}

function ApplyMaxMin(val, minVal, maxVal) {
  if (val > 0)
    return val * maxVal;
  else
    return val * minVal;
}

// G-forces from SimHub properties
/*
var gforce_x = $prop('ShakeITMotorsV3Plugin.Export.LateralGforce.Right')-$prop('ShakeITMotorsV3Plugin.Export.LateralGforce.Left')// acceleration
var gforce_y = $prop('ShakeITMotorsV3Plugin.Export.DecelGforce.Front') - $prop('ShakeITMotorsV3Plugin.Export.AccelGforce.Rear')// 
var gforce_z = 0;//
*/
var gforce_x = $prop('AccelerationSway');	// lateral (yaw) 
var gforce_y = - $prop('GlobalAccelerationG');	// deceleration
var gforce_z = $prop('AccelerationHeave');

gforce_x *= $prop('Settings.yaw_gain');
gforce_y *= $prop('Settings.decel_gain');
gforce_z *= $prop('Settings.heave_gain');


// if (0 > gforce_y)
//   gforce_y = 0;				// non-negative deceleration


var lsVal = 0;
var rsVal = 0;
var lwVal = 0;
var rwVal = 0;

// convert speed and yaw changes to left and right tension values
// turning right should increase right harness tension (body pushed left)
var rs = Math.sqrt(gforce_y * gforce_y + gforce_x * gforce_x);
var ls = gforce_y + gforce_y - rs;
if (0 > gforce_x) {
  var t = rs;	// negative gforce_x increases left tension
  rs = ls;
  ls = t;
}

var rw = Math.sqrt(gforce_z * gforce_z + gforce_x * gforce_x);
var lw = gforce_z + gforce_z - rw;
if (0 > gforce_x) {
  var t = rw;	// negative gforce_x increases left tension
  rw = lw;
  lw = t;
}

// Normalize to a value between 0 and 1
rsVal = rs / 200000;
lsVal = ls / 200000;
rwVal = rw / 200000;
lwVal = lw / 200000;

//return lsVal.toString()+","+lwVal.toString()+","+rwVal.toString()+","+rsVal.toString()




//return rsVal;

// // Assign values to belts
// // TODO: recombine the values the way you really want them. For now just put heave to waist for test
// lsVal = l;
// rsVal = r;


lsVal = SmoothItOut("ls", lsVal);
rsVal = SmoothItOut("rs", rsVal);
lwVal = SmoothItOut("lw", lwVal);
rwVal = SmoothItOut("rw", rwVal);


lsVal = ApplyMaxMin(lsMin, lsMax, lsVal);
rsVal = ApplyMaxMin(rsMin, rsMax, rsVal);
lwVal = ApplyMaxMin(lwMin, lwMax, lwVal);
rwVal = ApplyMaxMin(rwMin, rwMax, rwVal);

var command = "";
command = concatCommand(command, lsCtl, getCommandValFromAbs(applyNeutralOffset(lsnPos, lsVal)));
command = concatCommand(command, rsCtl, getCommandValFromAbs(applyNeutralOffset(rsnPos, rsVal)));
command = concatCommand(command, lwCtl, getCommandValFromAbs(applyNeutralOffset(lwnPos, lwVal)));
command = concatCommand(command, rwCtl, getCommandValFromAbs(applyNeutralOffset(rwnPos, rwVal)));
return command;
