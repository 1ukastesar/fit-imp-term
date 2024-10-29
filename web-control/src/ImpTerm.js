import React, { useState } from 'react';
import { Container, TextField, Button, Typography, Box, Alert, InputAdornment, OutlinedInput, InputLabel, FormControl} from '@mui/material';

const ImpTerm = () => {
  // State for input fields
  const [newPIN, setNewPIN] = useState('');
  const [confirmPIN, setPINconfirmation] = useState('');
  const [doorOpenDuration, setDoorOpenDuration] = useState('');

  // Handle submit
  const handleSubmit = (e) => {
    e.preventDefault();
    console.log('New access PIN:', newPIN);
    console.log('PIN confirmation:', confirmPIN);
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
            value={newPIN}
            onChange={(e) => setNewPIN(e.target.value)}
            required
            type="number"
          />
          <TextField
            label="PIN confirmation"
            variant="outlined"
            value={confirmPIN}
            onChange={(e) => setPINconfirmation(e.target.value)}
            required
            type="number"
          />
          <Typography variant="h6" gutterBottom>
            Door open duration
          </Typography>
          <FormControl sx={{ width: '25ch' }} variant="outlined">
            <InputLabel htmlFor="door-open-duration">Duration</InputLabel>
            <OutlinedInput
              label="Duration"
              // variant="outlined"
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
