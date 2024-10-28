import React, { useState } from 'react';
import { Container, TextField, Button, Typography, Box, Alert } from '@mui/material';

const ImpTerm = () => {
  // State for input fields
  const [newPIN, setNewPIN] = useState('');
  const [confirmPIN, setPINconfirmation] = useState('');

  // Handle submit
  const handleSubmit = (e) => {
    e.preventDefault();
    console.log('New access PIN:', newPIN);
    console.log('PIN confirmation:', confirmPIN);
  };

  const bluetoothAPISupported = navigator.bluetooth;

  return (
    <Container maxWidth="sm" style={{ marginTop: '50px' }}>
      <Box 
        component="form" 
        onSubmit={handleSubmit} 
        display="flex" 
        flexDirection="column" 
        gap={2}
      >
        <Typography variant="h4" align="center" gutterBottom>
          IMP Access Terminal
        </Typography>
        {bluetoothAPISupported ? (
        <>
          <TextField
            label="New access PIN"
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
