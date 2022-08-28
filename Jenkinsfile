pipeline {
  agent none
  
  stages {
    stage('Build') {
              agent {
        label 'BuildNode'
      }
      steps {
        sh '''~/.local/bin/pio run'''
      }
    }

  
    stage('Hardware test') {
      agent {
        label 'TestNode'
      }
      steps {
        sh '''~/.local/bin/pio run -e Classic_Test -t upload --upload-port /dev/ttyUSB0
python test_scripts/check.py'''
      }
    }
  }
}
