const char inicio[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Empotrados</title>
    <script src="https://cdn.tailwindcss.com"></script>
  </head>

  <body class="bg-[#0F0F0F] text-white px-4 py-9">
    <main
      class="max-w-lg mx-auto divide-y divide-white sm:mt-20 sm:bg-[#191919] sm:px-6 sm:py-9 sm:rounded-xl"
    >
      <div class="mb-7">
        <h2 class="text-3xl font-bold text-center">Estado actual: %ESTADO%</h2>
      </div>
      <form action="" id="form" class="pt-4 space-y-4">
        <div class="flex flex-col gap-2">
          <label for="codigo" class="">Codigo de desbloqueo</label>
          <input
            id="codigo"
            name="codigo"
            type="text"
            class="h-12 px-4 bg-transparent border border-white rounded-lg"
            required
          />
        </div>
        <button
          class="bg-[#990100] w-full rounded-lg h-12 disabled:bg-[#999999] hover:bg-[#660908] disabled:cursor-not-allowed disabled:opacity-50 text-lg font-medium"
          id="enviar"
        >
          Enviar
        </button>
      </form>
    </main>
    <script>
      const d = document
      const $form = d.getElementById('form'),
        $enviar = d.getElementById('enviar')
      const API_URL = 'http://192.168.0.3'
      d.addEventListener('DOMContentLoaded', e => {
        $form.addEventListener('submit', e => {
          e.preventDefault()
          fetch(`${API_URL}/code?codigo=${e.target.codigo.value}`, {
            method: 'GET',
            headers: {
              'Content-Type': 'application/json',
            },
          })
        })
      })
    </script>
  </body>
</html>)rawliteral";

const char bloqueo[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Empotrados</title>
    <script src="https://cdn.tailwindcss.com"></script>
  </head>

  <body class="bg-[#0F0F0F] text-white px-4 py-9">
    <main
      class="max-w-lg mx-auto divide-y divide-white sm:mt-20 sm:bg-[#191919] sm:px-6 sm:py-9 sm:rounded-xl"
    >
      <div class="mb-7">
        <h2 class="text-3xl font-bold text-center" id="estado">Estado actual: %ESTADO%</h2>
      </div>
      <div class="flex flex-col gap-4 pt-7">
        <button
          class="h-12 bg-transparent border-2 border-[#990100] font-medium rounded-lg text-lg hover:bg-[#990100] disabled:bg-[#999999] disabled:cursor-not-allowed disabled:opacity-50 disabled:border-[#999999]"
          id="bloquear"
        >
          Bloquear
        </button>
        <button
          class="h-12 bg-transparent border-2 border-[#06781F] font-medium rounded-lg text-lg hover:bg-[#06781F] disabled:bg-[#999999] disabled:cursor-not-allowed disabled:opacity-50 disabled:border-[#999999]"
          id="desbloquear"
        >
          Desbloquear
        </button>
      </div>
    </main>
    <script>
      const d = document
      const $bloquear = d.getElementById('bloquear'),
        $desbloquear = d.getElementById('desbloquear'), 
        $estado = d.getElementById('estado')
      const API_URL = 'http://192.168.0.3'
      let estado = '%ESTADO%'

      d.addEventListener('DOMContentLoaded', e => {
        $bloquear.addEventListener('click', e => {
          const res = fetch(`${API_URL}/bloquear`, {
            method: 'GET',
            headers: {
              'Content-Type': 'application/json',
            },
          })
          estado = "ocupado"
          $bloquear.disabled = true
          $desbloquear.disabled = false
          $estado.textContent = `Estado actual: ${estado}`
          console.log('bloquear')
        })

        $desbloquear.addEventListener('click', e => {
          const res = fetch(`${API_URL}/desbloquear`, {
            method: 'GET',
            headers: {
              'Content-Type': 'application/json',
            },
          })
          estado = "desocupado"
          $bloquear.disabled = false
          $desbloquear.disabled = true
          $estado.textContent = `Estado actual: ${estado}`
          console.log('desbloquear')
        })

        console.log('%ESTADO%')

        if (estado.toLowerCase() === 'desocupado') {
          $bloquear.disabled = false
          $desbloquear.disabled = true
        } else {
          $bloquear.disabled = true
          $desbloquear.disabled = false
        }
      })
    </script>
  </body>
</html>
)rawliteral";
const char contra[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Empotrados</title>
    <script src="https://cdn.tailwindcss.com"></script>
  </head>

  <body class="bg-[#0F0F0F] text-white px-4 py-9">
    <main
      class="max-w-lg mx-auto divide-y divide-white sm:mt-20 sm:bg-[#191919] sm:px-6 sm:py-9 sm:rounded-xl"
    >
      <div class="mb-7">
        <h2 class="text-3xl font-bold text-center">Estado actual: %ESTADO%</h2>
      </div>
      <form action="" id="form" class="pt-4 space-y-4">
        <div class="flex flex-col gap-2">
          <label for="password" class="">Nueva password</label>
          <input
            id="password"
            name="password"
            type="text"
            class="h-12 px-4 bg-transparent border border-white rounded-lg"
            required
          />
        </div>
        <button
          class="bg-[#990100] w-full rounded-lg h-12 disabled:bg-[#999999] hover:bg-[#660908] disabled:cursor-not-allowed disabled:opacity-50 text-lg font-medium"
        >
          Enviar
        </button>
      </form>
    </main>
    <script>
      const d = document
      const $form = d.getElementById('form')
      const API_URL = 'http://192.168.0.3'

      d.addEventListener('DOMContentLoaded', e => {
        $form.addEventListener('submit', e => {
          e.preventDefault()
          fetch(`${API_URL}/password?password=${e.target.password.value}`, {
            method: 'GET',
            headers: {
              'Content-Type': 'application/json',
            },
          })
        })
      })
    </script>
  </body>
</html>
)rawliteral";