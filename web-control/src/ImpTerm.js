import { Alert, Box, Button, Container, FormControl, InputAdornment, InputLabel, OutlinedInput, TextField, Typography } from '@mui/material';
import React, { useState } from 'react';

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

    console.log('Set access PIN:', pin);

    bluetoothAPI.requestDevice({
      // filters: [{ services: ['56841198-7683-4116-9353-26881ff0dc43'] }]
      acceptAllDevices: true,
      optionalServices: ['56841198-7683-4116-9353-26881ff0dc43']
    })
    .then(device => {
      console.log('Chosen device:', device.name);
      return device.gatt.connect();
    })
    .then(server => {
      console.log('Getting service...');
      return server.getPrimaryService('56841198-7683-4116-9353-26881ff0dc43');
    })
    .then(service => {
      console.log('Getting characteristic...');
      return service.getCharacteristic('b97c2777-b164-4f45-a039-60cdf4897a16');
    })
    .then(characteristic => {
      console.log('Writing value...');
      const pinArray = new Uint8Array([parseInt(pin)]);
      return characteristic.writeValue(pinArray);
    })
    .then(_ => {
      console.log('PIN set successfully');
      // Reset input fields
      setPin('');
      setPinConfirmation('');
    })
    .catch(error => {
      console.error('Error:', error);
      alert('Error: ' + error);
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
