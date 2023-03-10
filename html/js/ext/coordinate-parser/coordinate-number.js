// Generated by CoffeeScript 1.10.0
var CoordinateNumber;

CoordinateNumber = (function() {
  function CoordinateNumber(coordinateNumbers) {
    coordinateNumbers = this.normalizeCoordinateNumbers(coordinateNumbers);
    this.degrees = coordinateNumbers[0], this.minutes = coordinateNumbers[1], this.seconds = coordinateNumbers[2], this.milliseconds = coordinateNumbers[3];
    this.sign = this.normalizedSignOf(this.degrees);
    this.degrees = Math.abs(this.degrees);
  }

  CoordinateNumber.prototype.normalizeCoordinateNumbers = function(coordinateNumbers) {
    var currentNumber, i, j, len, normalizedNumbers;
    normalizedNumbers = [0, 0, 0, 0];
    for (i = j = 0, len = coordinateNumbers.length; j < len; i = ++j) {
      currentNumber = coordinateNumbers[i];
      normalizedNumbers[i] = parseFloat(currentNumber);
    }
    return normalizedNumbers;
  };

  CoordinateNumber.prototype.normalizedSignOf = function(number) {
    if (number >= 0) {
      return 1;
    } else {
      return -1;
    }
  };

  CoordinateNumber.prototype.detectSpecialFormats = function() {
    if (this.degreesCanBeSpecial()) {
      if (this.degreesCanBeMilliseconds()) {
        return this.degreesAsMilliseconds();
      } else if (this.degreesCanBeDegreesMinutesAndSeconds()) {
        return this.degreesAsDegreesMinutesAndSeconds();
      } else if (this.degreesCanBeDegreesAndMinutes()) {
        return this.degreesAsDegreesAndMinutes();
      }
    }
  };

  CoordinateNumber.prototype.degreesCanBeSpecial = function() {
    var canBe;
    canBe = false;
    if (!this.minutes && !this.seconds) {
      canBe = true;
    }
    return canBe;
  };

  CoordinateNumber.prototype.degreesCanBeMilliseconds = function() {
    var canBe;
    if (this.degrees > 909090) {
      canBe = true;
    } else {
      canBe = false;
    }
    return canBe;
  };

  CoordinateNumber.prototype.degreesAsMilliseconds = function() {
    this.milliseconds = this.degrees;
    return this.degrees = 0;
  };

  CoordinateNumber.prototype.degreesCanBeDegreesMinutesAndSeconds = function() {
    var canBe;
    if (this.degrees > 9090) {
      canBe = true;
    } else {
      canBe = false;
    }
    return canBe;
  };

  CoordinateNumber.prototype.degreesAsDegreesMinutesAndSeconds = function() {
    var newDegrees;
    newDegrees = Math.floor(this.degrees / 10000);
    this.minutes = Math.floor((this.degrees - newDegrees * 10000) / 100);
    this.seconds = Math.floor(this.degrees - newDegrees * 10000 - this.minutes * 100);
    return this.degrees = newDegrees;
  };

  CoordinateNumber.prototype.degreesCanBeDegreesAndMinutes = function() {
    var canBe;
    if (this.degrees > 360) {
      canBe = true;
    } else {
      canBe = false;
    }
    return canBe;
  };

  CoordinateNumber.prototype.degreesAsDegreesAndMinutes = function() {
    var newDegrees;
    newDegrees = Math.floor(this.degrees / 100);
    this.minutes = this.degrees - newDegrees * 100;
    return this.degrees = newDegrees;
  };

  CoordinateNumber.prototype.toDecimal = function() {
    var decimalCoordinate;
    decimalCoordinate = this.sign * (this.degrees + this.minutes / 60 + this.seconds / 3600 + this.milliseconds / 3600000);
    return decimalCoordinate;
  };

  return CoordinateNumber;

})();

//module.exports = CoordinateNumber;