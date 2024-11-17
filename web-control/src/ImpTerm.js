import { Alert, Box, Button, Container, FormControl, InputAdornment, InputLabel, OutlinedInput, TextField, Typography } from '@mui/material';
import React, { useState } from 'react';
import { toast } from 'react-toastify';

// BLE service and characteristic UUIDs
const impTermSvcUuid = "automation_io";
const accessPinChrUuid = 'bf6036dc-5b62-425e-bed4-9b7f6ba1c921';
const doorOpenDurationChrUuid = '4e3ee180-27a0-4894-815a-c98a07ba1555';

// Convenience definitions
const UINT16_MAX = Math.pow(2, 16) - 1;
const bluetoothAPI = navigator.bluetooth;

/**
 * Convert a number to a Uint8Array
 * @param {number} num The number to convert
 * @returns {Uint8Array} The converted number
 * @example numToUint8Array(0) => Uint8Array(2) [0, 0]
 * @example numToUint8Array(1) => Uint8Array(2) [1, 0]
 * @example numToUint8Array(256) => Uint8Array(2) [0, 1]
 * @example numToUint8Array(65535) => Uint8Array(2) [255, 255]
 * @example numToUint8Array(1234) => Uint8Array(2) [210, 4]
 * @cite Peschel, R. (2022). "How to convert a Javascript number to a Uint8Array?" Stack Overflow. Available at: https://stackoverflow.com/a/72476502 [Accessed 17 Nov. 2024].
*/
function numToUint8Array(num) {
  let arr = new Uint8Array(2);

  for (let i = 0; i < 8; i++) {
    arr[i] = num % 256;
    num = Math.floor(num / 256);
  }

  return arr;
}

/**
 * Check if the PIN is a valid 4-10 digit number
 * @param {string} pin The PIN to validate
 * @returns {boolean} True if the PIN is valid, false otherwise
 * @example pinValid('1234') => true
 * @example pinValid('123') => false
 * @example pinValid('12345') => true
 * @example pinValid('12345a') => false
*/
const pinValid = (pin) => {
  const pinFormat = /^[0-9]{4,10}$/;
  return pinFormat.test(pin);
};

class ConnectionAborted extends Error {}

/**
 * Get the Bluetooth device
 * @returns {Promise<BluetoothDevice>} The Bluetooth device
 */
const handleConnection = async (notification) => {
  try {
    if (impTermDevice.gatt.connected)
        console.log('Device already connected')
    return impTermDevice.gatt.connect();
  } catch(error) {
    try {
      impTermDevice = await bluetoothAPI.requestDevice({
        acceptAllDevices: true,
        optionalServices: [impTermSvcUuid]
      });
      console.log('Chosen device:', impTermDevice.name);
      return impTermDevice.gatt.connect()
    }
    catch(error) {
      if(error.message.includes('User cancelled')) {
        console.log('Connection cancelled');
        toast.update(notification, { render: "Connection cancelled", type: "warning", isLoading: false, autoClose: true });
      }
      else {
        console.error('Error:', error);
        toast.error('An error occurred');
      }
      throw new ConnectionAborted();
    }
  }
}

const handleChangeError = (error, notification) => {
  if(error.message.includes('GATT operation not permitted')) {
    toast.update(notification, { render: "Operation not permitted", type: "error", isLoading: false, autoClose: true });
  }
  else {
    console.error('Error:', error);
    toast.update(notification, { render: "An error occurred", type: "error", isLoading: false, autoClose: true });
  }
}

// Global variable to store the connected device
var impTermDevice = null;

const ImpTerm = () => {
  // State for input fields
  const [pin, setPin] = useState('');
  const [pinConfirmation, setPinConfirmation] = useState('');
  const [doorOpenDuration, setDoorOpenDuration] = useState('');

  const [isPinValid, setPinValidity] = useState(true);
  const [isPinConfirmationValid, setPinConfirmationValidity] = useState(true);
  const [pinHelper, setPinHelper] = useState('');
  const [pinConfirmationHelper, setPinConfirmationHelper] = useState('');

  const checkPinValid = () => {
    checkPinsMatch();
    if(!pinValid(pin) && pin.length > 0) {
      setPinValidity(false);
      setPinHelper('PIN must be a 4-10 digit number');
      return false;
    }
    return true;
  }

  const checkPinsMatch = () => {
    if(pin !== pinConfirmation) {
      setPinConfirmationValidity(false);
      setPinConfirmationHelper('PINs do not match');
      return false;
    }
    return true;
  }

  const clearPinFlags = () => {
    setPinValidity(true);
    setPinHelper('');
    clearConfirmationFlags();
  }

  const clearConfirmationFlags = () => {
    setPinConfirmationValidity(true);
    setPinConfirmationHelper('');
  }

  const handlePinSubmit = (e) => {
    e.preventDefault();

    if(!checkPinValid())
      return;

    if(!checkPinsMatch())
      return;

    const pinChangeToast = toast.loading("PIN change pending...")

    console.log('Requested PIN change:', pin);
    const textEncoder = new TextEncoder();
    const pinConvUint8 = textEncoder.encode(pin);
    console.log('Converted PIN:', pinConvUint8);

    handleConnection(pinChangeToast)
    .then(server => {
      console.log('Getting service...');
      return server.getPrimaryService(impTermSvcUuid);
    })
    .then(service => {
      console.log('Getting characteristic...');
      return service.getCharacteristic(accessPinChrUuid);
    })
    .then(characteristic => {
      console.log('Writing value...');
      console.log('PIN array:', pinConvUint8);
      return characteristic.writeValue(pinConvUint8);
    })
    .then(_ => {
      console.log('PIN set successfully');
      // Reset input fields
      setPin('');
      setPinConfirmation('');
      toast.update(pinChangeToast, { render: "PIN updated", type: "success", isLoading: false, autoClose: true });
    })
    .catch(error => {
      if(error instanceof ConnectionAborted)
        return;
      handleChangeError(error, pinChangeToast);
    });
  };

  const handleDurationSubmit = (e) => {
    e.preventDefault();

    const durationChangeToast = toast.loading("Duration change pending...")
    console.log('Requested door open duration change:', doorOpenDuration);

    const durationConvUint8 = numToUint8Array(doorOpenDuration);
    console.log('Converted duration:', durationConvUint8);

    handleConnection(durationChangeToast)
    .then(server => {
      console.log('Getting service...');
      return server.getPrimaryService(impTermSvcUuid);
    })
    .then(service => {
      console.log('Getting characteristic...');
      return service.getCharacteristic(doorOpenDurationChrUuid);
    })
    .then(characteristic => {
      console.log('Writing value...');
      console.log('Duration array:', durationConvUint8);
      return characteristic.writeValue(durationConvUint8);
    })
    .then(_ => {
      console.log('Duration set successfully');
      // Reset input fields
      setDoorOpenDuration('');
      toast.update(durationChangeToast, { render: "Duration updated", type: "success", isLoading: false, autoClose: true });
    })
    .catch(error => {
      if(error instanceof ConnectionAborted)
        return;
      handleChangeError(error, durationChangeToast);
    });
  }

  return (
    <Container maxWidth="sm" style={{ marginTop: '50px' }}>
      <Typography variant="h4" align="center" gutterBottom>
        IMP Access Terminal
      </Typography>
        {bluetoothAPI ? (
        <>
          <br />
          <Box
          component="form"
          onSubmit={handlePinSubmit}
          display="flex"
          flexDirection="column"
          gap={2}
          >
            <Typography variant="h6" gutterBottom>
              Access PIN
            </Typography>
            <TextField
              label="New PIN"
              variant="outlined"
              id="new-pin"
              value={pin}
              onChange={(e) => setPin(e.target.value)}
              onFocus={() => clearPinFlags()}
              onBlur={() => checkPinValid()}
              error={!isPinValid}
              required
              type="number"
              helperText={pinHelper}
            />
            <TextField
              label="PIN confirmation"
              variant="outlined"
              id="confirm-pin"
              value={pinConfirmation}
              onChange={(e) => setPinConfirmation(e.target.value)}
              onFocus={() => clearConfirmationFlags()}
              onBlur={() => checkPinsMatch()}
              error={!isPinConfirmationValid}
              required
              type="number"
              helperText={pinConfirmationHelper}
            />
            <Button
              type="submit"
              variant="contained"
              color="primary"
              fullWidth
            >
              Change
            </Button>
          </Box>
          <br />
          <Box
            component="form"
            onSubmit={handleDurationSubmit}
            display="flex"
            flexDirection="column"
            gap={2}
          >
            <Typography variant="h6" gutterBottom>
              Door open duration
            </Typography>
            <FormControl variant="outlined">
              <InputLabel htmlFor="door-open-duration">Duration</InputLabel>
              <OutlinedInput
                label="Duration"
                id="door-open-duration"
                value={doorOpenDuration}
                onChange={(e) => setDoorOpenDuration(e.target.value)}
                required
                type="number"
                endAdornment={<InputAdornment position="end">s</InputAdornment>}
                placeholder='10'
                inputProps={{ min: 1, max: UINT16_MAX }}
              />
            </FormControl>
            <Button
              type="submit"
              variant="contained"
              color="primary"
              fullWidth
            >
              Update
            </Button>
          </Box>
        </>
        ) : (
          <>
            <br />
            <Alert severity="error">
              <b>Web Bluetooth API is not supported in this browser.</b>
            </Alert>
            <br />
            <Alert severity="info">
              Use the latest version of Chrome or Edge and enable the experimental Web Platform features flag:
              <br />
              <a href="chrome://flags/#enable-experimental-web-platform-features">chrome://flags/#enable-experimental-web-platform-features</a>
              <br />
              <br />
              You need to enter that URL manually, as Chrome does not allow direct links to internal pages. Restart the browser for the setting to take effect.
            </Alert>
          </>
        )}
    </Container>
  );
};

export default ImpTerm;
