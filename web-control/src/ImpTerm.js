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
    if(!pinValid(pin)) {
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
  }

  const clearConfirmationFlags = () => {
    setPinConfirmationValidity(true);
    setPinConfirmationHelper('');
  }

  // Handle submit
  const handleSubmit = (e) => {
    e.preventDefault();

    checkPinValid();

    if(!checkPinsMatch())
      return;

    console.log('Set access PIN:', pin);
    console.log('Set door open duration:', doorOpenDuration);

    // Reset input fields
    setPin('');
    setPinConfirmation('');
    setDoorOpenDuration('');
  };

  const bluetoothAPISupported = navigator.bluetooth;

  return (
    <Container maxWidth="sm" style={{ marginTop: '50px' }}>
      <Typography variant="h4" align="center" gutterBottom>
        IMP Access Terminal
      </Typography>
      <Box 
        component="form"
        onSubmit={handleSubmit}
        display="flex"
        flexDirection="column"
        gap={2}
      >
        {bluetoothAPISupported ? (
        <>
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
            min='1'
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
            min='1'
          />
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
      </Box>
    </Container>
  );
};

export default ImpTerm;
