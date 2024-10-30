import React, { useState } from 'react';
import { Container, TextField, Button, Typography, Box, Alert, InputAdornment, OutlinedInput, InputLabel, FormControl} from '@mui/material';

const ImpTerm = () => {
  // State for input fields
  const [pin, setPin] = useState('');
  const [pinConfirmation, setPinConfirmation] = useState('');
  const [doorOpenDuration, setDoorOpenDuration] = useState('');

  const [isPinValid, setPinValidity] = useState(true);
  const [isPinConfirmationValid, setPinConfirmationValidity] = useState(true);
  const [pinHelper, setPinHelper] = useState('');
  const [pinConfirmationHelper, setPinConfirmationHelper] = useState('');

  const pinValid = (pin) => {
    const pinFormat = /^[0-9]{3}[0-9]+$/;
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

    // Reset input fields
    setPin('');
    setPinConfirmation('');
  };

  const handleDurationSubmit = (e) => {
    e.preventDefault();

    console.log('Set door open duration:', doorOpenDuration);

    // Reset input fields
    setDoorOpenDuration('');
  }

  const bluetoothAPISupported = navigator.bluetooth;

  return (
    <Container maxWidth="sm" style={{ marginTop: '50px' }}>
      <Typography variant="h4" align="center" gutterBottom>
        IMP Access Terminal
      </Typography>
        {bluetoothAPISupported ? (
        <>
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
            <FormControl variant="outlined">
              <InputLabel htmlFor="new-pin">New PIN</InputLabel>
              <OutlinedInput
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
            </FormControl>
            <FormControl variant="outlined">
              <InputLabel htmlFor="confirm-pin">PIN confirmation</InputLabel>
              <OutlinedInput
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
            <Alert severity="error">
              <b>Web Bluetooth API is not supported in this browser.</b>
            </Alert>
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
