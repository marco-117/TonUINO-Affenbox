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
        sh '''I could be malicious code running in your private hardware and network...'''
      }
    }
  }
}
