var lsn = [$prop('Settings.LeftShoulderNeutral')];
var rsn = [$prop('Settings.RightShoulderNeutral')];
var lwn = [$prop('Settings.LeftWaistNeutral')];
var rwn = [$prop('Settings.RightWaistNeutral')];

var lsm = [$prop('Settings.LeftShoulderMax')];
var rsm = [$prop('Settings.RightShoulderMax')];
var lwm = [$prop('Settings.LeftWaistMax')];
var rwm = [$prop('Settings.RightWaistMax')];

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

// G-forces from SimHub properties
var Gy = - $prop('AccelerationSway');	// lateral (yaw) acceleration
var Gs = - $prop('GlobalAccelerationG');	// deceleration
var tmax = $prop('Settings.tmax') & 126;	// limit servos
Gy *= $prop('Settings.yaw_gain');
Gs *= $prop('Settings.decel_gain');
if (0 > Gs)
  Gs = 0;				// non-negative deceleration
/*  
else if (tmax < Gs)
  $prop('Settings.decel_gain') *= Math.round(tmax/Gs - 0.5);	// unsupported..
 */
// convert speed and yaw changes to left and right tension values
// turning right should increase right harness tension (body pushed left)
var r = Math.sqrt(Gs*Gs + Gy*Gy);
var l = Gs + Gs - r;
if (0 > Gy) {
  var t = r;	// negative Gy increases left tension
  r = l;
  l = t;
}

// Low-pass IIR filtering of left and right tension values
if (null == root["lb4"]) {
  root["rb4"] = r;  root["lb4"] = l;	// initialize
}
var rb4 = root["rb4"];
var lb4 = root["lb4"]; // previously filtered values
var tc = 1 + $prop('Settings.smooth');
rb4 += (r - rb4) / tc;
lb4 += (l - lb4) / tc;
root["lb4"] = lb4;
root["rb4"] = rb4;

l = lb4; r = rb4; // filtered tensions;  comment out for unfiltered (or set Settings.smooth = 1)
if (l > tmax)
  l = tmax;
else if (l < 2)
  l = 2;
l &= 0x7E;      // left lsb is 0
tmax |= 1;
if (r > tmax)
  r = tmax;
else if (r < 3)
  r = 3;
r |= 1;         // right lsb is 1

if ($prop('Settings.max_test') || $prop('Settings.TestOffsets')) {
  // disable normal message output
  // slider changes will provoke first message outputs
  root["rb4"] = root["lb4"] = 0;  // reset IIR filters
} else {
//* servo control output
  var ls = String.fromCharCode(l);      // tension control characters
  var rs = String.fromCharCode(r);
//*/

/* gnuplot output **************************************
  var s = $prop('SpeedMph');
  var ls = l.toString();
  var rs = r.toString();
  var ss = s.toString();
  var Gys = Gy.toString();
  var Gss = Gs.toString();
  rs = ls.concat('\	',rs,'\\\n');  // gnuplot columns
  ls = ss.concat('\	'); 
*/
return ls.concat(rs);
}