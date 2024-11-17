import { Alert, Box, Button, Container, FormControl, InputAdornment, InputLabel, OutlinedInput, TextField, Typography } from '@mui/material';
import React, { useState } from 'react';
import { toast } from 'react-toastify';

import 'react-toastify/dist/ReactToastify.css';

const impTermSvcUuid = "automation_io";
const accessPinChrUuid = 'bf6036dc-5b62-425e-bed4-9b7f6ba1c921';
const doorOpenDurationChrUuid = '4e3ee180-27a0-4894-815a-c98a07ba1555';

const ImpTerm = () => {
  // State for input fields
  const [pin, setPin] = useState('');
  const [pinConfirmation, setPinConfirmation] = useState('');
  const [doorOpenDuration, setDoorOpenDuration] = useState('');

  const [isPinValid, setPinValidity] = useState(true);
  const [isPinConfirmationValid, setPinConfirmationValidity] = useState(true);
  const [pinHelper, setPinHelper] = useState('');
  const [pinConfirmationHelper, setPinConfirmationHelper] = useState('');

  const bluetoothAPI = navigator.bluetooth;

  const pinValid = (pin) => {
    const pinFormat = /^[0-9]{4,10}$/;
    return pinFormat.test(pin);
  };

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

    bluetoothAPI.requestDevice({
      acceptAllDevices: true,
      optionalServices: [impTermSvcUuid]
    })
    .then(device => {
      console.log('Chosen device:', device.name);
      return device.gatt.connect();
    })
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
      console.error('Error:', error);
      if(error.message.includes('GATT operation not permitted'))
        toast.update(pinChangeToast, { render: "Operation not permitted", type: "error", isLoading: false, autoClose: true });
      else if(error.message.includes('User cancelled'))
        toast.update(pinChangeToast, { render: "Operation cancelled", type: "warning", isLoading: false, autoClose: true });
      else
      toast.update(pinChangeToast, { render: "An error occurred", type: "error", isLoading: false, autoClose: true });
    });
  };

  const handleDurationSubmit = (e) => {
    e.preventDefault();

    console.log('Set door open duration:', doorOpenDuration);

    // Reset input fields
    setDoorOpenDuration('');
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
              />
            </FormControl>
            <Button
              type="submit"
              variant="contained"
              color="primary"
              fullWidth
            >
              Change
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
