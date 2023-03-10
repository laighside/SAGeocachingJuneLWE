// Generated by CoffeeScript 1.10.0
var Validator;

Validator = (function() {
  function Validator() {}

  Validator.prototype.isValid = function(coordinates) {
    var error, isValid, validationError;
    isValid = true;
    try {
      this.validate(coordinates);
      return isValid;
    } catch (error) {
      validationError = error;
      isValid = false;
      return isValid;
    }
  };

  Validator.prototype.validate = function(coordinates) {
    this.checkContainsNoLetters(coordinates);
    this.checkValidOrientation(coordinates);
    return this.checkNumbers(coordinates);
  };

  Validator.prototype.checkContainsNoLetters = function(coordinates) {
    var containsLetters;
    containsLetters = /(?![neswd])[a-z]/i.test(coordinates);
    if (containsLetters) {
      throw new Error('Coordinate contains invalid alphanumeric characters.');
    }
  };

  Validator.prototype.checkValidOrientation = function(coordinates) {
    var validOrientation;
    validOrientation = /^[^nsew]*[ns]?[^nsew]*[ew]?[^nsew]*$/i.test(coordinates);
    if (!validOrientation) {
      throw new Error('Invalid cardinal direction.');
    }
  };

  Validator.prototype.checkNumbers = function(coordinates) {
    var coordinateNumbers;
    coordinateNumbers = coordinates.match(/-?\d+(\.\d+)?/g);
    this.checkAnyCoordinateNumbers(coordinateNumbers);
    this.checkEvenCoordinateNumbers(coordinateNumbers);
    return this.checkMaximumCoordinateNumbers(coordinateNumbers);
  };

  Validator.prototype.checkAnyCoordinateNumbers = function(coordinateNumbers) {
    if (coordinateNumbers.length === 0) {
      throw new Error('Could not find any coordinate number');
    }
  };

  Validator.prototype.checkEvenCoordinateNumbers = function(coordinateNumbers) {
    var isUnevenNumbers;
    isUnevenNumbers = coordinateNumbers.length % 2;
    if (isUnevenNumbers) {
      throw new Error('Uneven count of latitude/longitude numbers');
    }
  };

  Validator.prototype.checkMaximumCoordinateNumbers = function(coordinateNumbers) {
    if (coordinateNumbers.length > 6) {
      throw new Error('Too many coordinate numbers');
    }
  };

  return Validator;

})();

//module.exports = Validator;
