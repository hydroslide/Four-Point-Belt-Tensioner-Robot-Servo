var utils = GetTensionUtils();

var command = "";

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

function GetGForceCommand() {

  // G-forces from SimHub properties
  /*
  var gforce_x = $prop('ShakeITMotorsV3Plugin.Export.LateralGforce.Right')-$prop('ShakeITMotorsV3Plugin.Export.LateralGforce.Left')// acceleration
  var gforce_y = $prop('ShakeITMotorsV3Plugin.Export.DecelGforce.Front') - $prop('ShakeITMotorsV3Plugin.Export.AccelGforce.Rear')// 
  var gforce_z = 0;//
  */
  var gForceSway = $prop('AccelerationSway')*2.65;//1.75;	// lateral (yaw) 
  var gForceSurge = $prop('GlobalAccelerationG');	// deceleration
  var gForceHeave = $prop('AccelerationHeave');
  if ($prop('DataCorePlugin.CurrentGame') == "CodemastersDirtRally2")
    gForceHeave = $prop('ShakeITMotorsV3Plugin.Export.jumpLanding.All') * -.1;


  var masterGain = $prop('Settings.master_gain') / 10;

  gForceSway *= ($prop('Settings.yaw_gain') / 10) * masterGain;
  gForceSurge *= ($prop('Settings.decel_gain') / 10) * masterGain;
  gForceHeave *= ($prop('Settings.heave_gain') / 10) * masterGain;

  //if ($prop('DataCorePlugin.CurrentGame') == "AssettoCorsaCompetizione")
  //  gForceSway *= 1.5



  var lsVal = 0;
  var rsVal = 0;
  var lwVal = 0;
  var rwVal = 0;

  var rs = 0;
  var ls = 0;
  var rw = 0;
  var lw = 0;

  var beltForces = {
    ls: [],
    rs: [],
    lw: [],
    rw: []
  }

  var combineForces = function (arr) {
    //return avg(arr);
    var bestValue = 0;

    arr.forEach(function (val) {
      if (Math.abs(val) > Math.abs(bestValue))
        bestValue = val;
    });
    return bestValue;
  }

  var avg = function (arr) {
    if (arr.length > 0) {
      var total = 0;

      arr.forEach(function (val) {
        total += val;
      });

      return total / arr.length;
    }
    else
      return 0;
  }

  var positiveSurgeMultiplier = 2;//1.5

  if (gForceSurge > 0)
    gForceSurge *= positiveSurgeMultiplier;

  beltForces.ls.push(gForceSurge);
  beltForces.rs.push(gForceSurge);
  if (gForceSurge < 0) {
    beltForces.lw.push(Math.abs(gForceSurge))
    beltForces.rw.push(Math.abs(gForceSurge))
  }

  var shoulderSwayMultiplier = .25
  if (gForceSway < 0) {
    beltForces.lw.push(gForceSway * -1);
    beltForces.ls.push(gForceSway * -1 * shoulderSwayMultiplier);
  }
  else {
    beltForces.rw.push(gForceSway);
    beltForces.rs.push(gForceSway * shoulderSwayMultiplier);
  }

  beltForces.lw.push(-1 * gForceHeave);
  beltForces.rw.push(-1 * gForceHeave);

  ls = combineForces(beltForces.ls);
  rs = combineForces(beltForces.rs);
  lw = combineForces(beltForces.lw);
  rw = combineForces(beltForces.rw);

  // Normalize to a value between 0 and 1
  rsVal = rs / 2000;
  lsVal = ls / 2000;
  rwVal = rw / 2000;
  lwVal = lw / 2000;

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


  lsVal = ApplyMaxMin(utils.lsMin, utils.lsMax, lsVal);
  rsVal = ApplyMaxMin(utils.rsMin, utils.rsMax, rsVal);
  lwVal = ApplyMaxMin(utils.lwMin, utils.lwMax, lwVal);
  rwVal = ApplyMaxMin(utils.rwMin, utils.rwMax, rwVal);

  var concatCommand = utils.concatCommand;
  var getCommandValFromAbs = utils.getCommandValFromAbs;
  var applyNeutralOffset = utils.applyNeutralOffset;

  var command = "";


  if (!$prop('Settings.useShoulder')) {
    lsVal = 0;
    rsVal = 0;
  }
  if (!$prop('Settings.useWaist')) {
    lwVal = 0;
    rwVal = 0;
  }
  command = concatCommand(command, utils.ctlCodes.leftShoulder, getCommandValFromAbs(applyNeutralOffset(utils.lsnPos, lsVal)));
  command = concatCommand(command, utils.ctlCodes.rightShoulder, getCommandValFromAbs(applyNeutralOffset(utils.rsnPos, rsVal)));
  command = concatCommand(command, utils.ctlCodes.leftWaist, getCommandValFromAbs(applyNeutralOffset(utils.lwnPos, lwVal)));
  command = concatCommand(command, utils.ctlCodes.rightWaist, getCommandValFromAbs(applyNeutralOffset(utils.rwnPos, rwVal)));

  return command;
}

function checkForLimit() {
  if (!$prop('Settings.forceCalibrationNow')) {
    root['calibrationForced'] = false;
  }
  if (root['limitFound'] != true) {
    command = command.concat(String.fromCharCode(utils.ctlCodes.control));
    command = command.concat(String.fromCharCode(utils.ctlCodes.findTheLimitsCtl));
    root['limitFound'] = true;
    return command;
  } else
    return null;
}

function SetNeutralPositionsCommands() {
  command = utils.concatCommand(command, utils.ctlCodes.leftShoulderNeutral, utils.getCommandValFromAbs(utils.lsnPos));
  command = utils.concatCommand(command, utils.ctlCodes.rightShoulderNeutral, utils.getCommandValFromAbs(utils.rsnPos));
  command = utils.concatCommand(command, utils.ctlCodes.leftWaistNeutral, utils.getCommandValFromAbs(utils.lwnPos));
  command = utils.concatCommand(command, utils.ctlCodes.rightWaistNeutral, utils.getCommandValFromAbs(utils.rwnPos));
  root['leftShoulderNeutral'] = utils.lsnPos;
  root['rightShoulderNeutral'] = utils.rsnPos;
  root['leftWaistNeutral'] = utils.lwnPos;
  root['rightWaistNeutral'] = utils.rwnPos;
  return command;
}

function ApplyNeutralPosition() {
  command = utils.concatCommand(command, utils.ctlCodes.leftShoulder, utils.getCommandValFromAbs(utils.lsnPos));
  command = utils.concatCommand(command, utils.ctlCodes.rightShoulder, utils.getCommandValFromAbs(utils.rsnPos));
  command = utils.concatCommand(command, utils.ctlCodes.leftWaist, utils.getCommandValFromAbs(utils.lwnPos));
  command = utils.concatCommand(command, utils.ctlCodes.rightWaist, utils.getCommandValFromAbs(utils.rwnPos));
  return command;
}

function runInit() {
  root['init'] = true;
  root['limitFound'] = false;
  return SetNeutralPositionsCommands();
}

if (root['init'] != true)
  return runInit();
else if (root['leftShoulderNeutral'] != utils.lsnPos || root['rightShoulderNeutral'] != utils.rsnPos || root['leftWaistNeutral'] != utils.lwnPos || root['rightWaistNeutral'] != utils.rwnPos) {
  root['init'] = false;
  return "";
} else if ($prop('Settings.TestMaxTensions')) {
  command = utils.concatCommand(command, utils.ctlCodes.leftShoulder, utils.getCommandValFromAbs(utils.getMaxTensionPos(utils.lsnPos, utils.lsMax)));
  command = utils.concatCommand(command, utils.ctlCodes.rightShoulder, utils.getCommandValFromAbs(utils.getMaxTensionPos(utils.rsnPos, utils.rsMax)));
  command = utils.concatCommand(command, utils.ctlCodes.leftWaist, utils.getCommandValFromAbs(utils.getMaxTensionPos(utils.lwnPos, utils.lwMax)));
  command = utils.concatCommand(command, utils.ctlCodes.rightWaist, utils.getCommandValFromAbs(utils.getMaxTensionPos(utils.rwnPos, utils.rwMax)));
  return command;
} else if ($prop('Settings.TestMinTensions')) {
  command = utils.concatCommand(command, utils.ctlCodes.leftShoulder, utils.getCommandValFromAbs(utils.getMinTensionPos(utils.lsnPos, utils.lsMin)));
  command = utils.concatCommand(command, utils.ctlCodes.rightShoulder, utils.getCommandValFromAbs(utils.getMinTensionPos(utils.rsnPos, utils.rsMin)));
  command = utils.concatCommand(command, utils.ctlCodes.leftWaist, utils.getCommandValFromAbs(utils.getMinTensionPos(utils.lwnPos, utils.lwMin)));
  command = utils.concatCommand(command, utils.ctlCodes.rightWaist, utils.getCommandValFromAbs(utils.getMinTensionPos(utils.rwnPos, utils.rwMin)));
  return command;
} if ($prop('Settings.TestNeutralTensions')) {
  return ApplyNeutralPosition();
} else if ($prop('Settings.forceCalibrationNow')) {
  if (root['calibrationForced'] != true) {
    root['limitFound'] = false;
    root['calibrationForced'] = true;
  }
  return "Must Force Calibration";
} else if ($prop('DataCorePlugin.GameRunning') != 0) {
  var limitCommand = checkForLimit();
  if (limitCommand)
    return limitCommand
  else
    return GetGForceCommand();
} else {
  var limitCommand = checkForLimit()
  if (limitCommand)
    return limitCommand
  else {
    if ($prop('Settings.forceCalibration'))
      root['limitFound'] = false;
    return ApplyNeutralPosition();
  }
}
